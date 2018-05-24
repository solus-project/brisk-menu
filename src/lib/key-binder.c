/*
 * This file is part of brisk-menu.
 *
 * Largely inspired by: https://esite.ch/2011/02/global-hotkeys-with-vala/
 *
 * Copyright © 2016-2018 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "key-binder.h"
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

struct _BriskKeyBinderClass {
        GObjectClass parent_class;
};

/**
 * BriskKeyBinder is used to bind global x11 shortcuts
 */
struct _BriskKeyBinder {
        GObject parent;
        GdkWindow *root_window;
        GHashTable *bindings;
        gboolean wait_for_release;
};

/**
 * A KeyBinding is used to map the input + function pointer into something
 * we can store and test.
 */
typedef struct KeyBinding {
        const gchar *accelerator;
        KeyCode keycode;
        GdkModifierType mods;
        BinderFunc func;
        gpointer udata;
} KeyBinding;

static GdkFilterReturn brisk_key_binder_filter(GdkXEvent *xevent, GdkEvent *event, gpointer v);
static void free_keybinding(KeyBinding *binding);

/**
 * When binding a shortcut, we bind all possible combinations of lock key modifiers
 * so that we'll get the event regardless (i.e. capslock/numlock)
 */
static GdkModifierType _modifiers[] = { 0,
                                        GDK_MOD2_MASK,
                                        GDK_LOCK_MASK,
                                        GDK_MOD5_MASK,
                                        GDK_MOD2_MASK | GDK_LOCK_MASK,
                                        GDK_MOD2_MASK | GDK_MOD5_MASK,
                                        GDK_LOCK_MASK | GDK_MOD5_MASK,
                                        GDK_MOD2_MASK | GDK_LOCK_MASK | GDK_MOD5_MASK };

G_DEFINE_TYPE(BriskKeyBinder, brisk_key_binder, G_TYPE_OBJECT)

/**
 * brisk_key_binder_new:
 *
 * Construct a new BriskKeyBinder object
 */
BriskKeyBinder *brisk_key_binder_new()
{
        return g_object_new(BRISK_TYPE_KEY_BINDER, NULL);
}

/**
 * brisk_key_binder_dispose:
 *
 * Clean up a BriskKeyBinder instance
 */
static void brisk_key_binder_dispose(GObject *obj)
{
        BriskKeyBinder *self = NULL;

        /* Remove our filter again */
        self = BRISK_KEY_BINDER(obj);
        if (self->root_window) {
                gdk_window_remove_filter(self->root_window, brisk_key_binder_filter, self);
                self->root_window = NULL;
        }

        /* Will automatically clean up all bindings too */
        g_clear_pointer(&self->bindings, g_hash_table_unref);

        G_OBJECT_CLASS(brisk_key_binder_parent_class)->dispose(obj);
}

/**
 * brisk_key_binder_class_init:
 *
 * Handle class initialisation
 */
static void brisk_key_binder_class_init(BriskKeyBinderClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_key_binder_dispose;
}

/**
 * brisk_key_binder_init:
 *
 * Handle construction of the BriskKeyBinder
 */
static void brisk_key_binder_init(BriskKeyBinder *self)
{
        GdkWindow *root = NULL;

        self->bindings =
            g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)free_keybinding);

        root = gdk_get_default_root_window();
        if (!root) {
                g_warning("No root window found, cannot bind events");
                return;
        }
        self->root_window = root;
        gdk_window_add_filter(root, brisk_key_binder_filter, self);
}

static char* get_window_manager(void)
{
        Window *xwindow;
        Window wm_window;
        Atom type;
        gint format;
        gulong nitems;
        gulong bytes_after;

        Atom wm_check = gdk_x11_get_xatom_by_name ("_NET_SUPPORTING_WM_CHECK");

        XGetWindowProperty (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), GDK_ROOT_WINDOW (),
                            wm_check, 0, G_MAXLONG, False, XA_WINDOW, &type, &format,
                            &nitems, &bytes_after, (guchar **) &xwindow);

        if (type != XA_WINDOW)
        {
                return g_strdup ("Unknown");
        }

        gdk_error_trap_push ();
        XSelectInput (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), *xwindow,
                      StructureNotifyMask | PropertyChangeMask);
        XSync (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), False);

        if (gdk_error_trap_pop ())
        {
                XFree (xwindow);
                return g_strdup ("Unknown");
        }

        wm_window = *xwindow;
        XFree (xwindow);

        Atom utf8_string = gdk_x11_get_xatom_by_name ("UTF8_STRING");
        Atom wm_name = gdk_x11_get_xatom_by_name ("_NET_WM_NAME");
        gchar *val;
        gchar *wm;
        int result;

        val = NULL;

        gdk_error_trap_push ();
        result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY(gdk_display_get_default()),
                                     wm_window,
                                     wm_name,
                                     0, G_MAXLONG,
                                     False, utf8_string,
                                     &type, &format, &nitems,
                                     &bytes_after, (guchar **) &val);

        if (gdk_error_trap_pop () || result != Success || type != utf8_string || format != 8 ||
            nitems == 0 || !g_utf8_validate (val, (gssize)nitems, NULL))
        {
                wm = NULL;
        }
        else
        {
                wm = g_strndup (val, nitems);
        }

        if (val)
                XFree (val);

        return wm;
}

/**
 * Handle global events (eventually)
 */
static GdkFilterReturn brisk_key_binder_filter(GdkXEvent *xevent, GdkEvent *event, gpointer v)
{
        BriskKeyBinder *self = NULL;
        XEvent *xev = xevent;
        GHashTableIter iter = { 0 };
        const gchar *key = NULL;
        const KeyBinding *binding = NULL;
        guint mods;
        Display *display = NULL;
        KeySym keysym;

        self = BRISK_KEY_BINDER(v);
        display = GDK_WINDOW_XDISPLAY(self->root_window);

        if (xev->type != KeyRelease && xev->type != KeyPress && xev->type != ButtonPress) {
                return GDK_FILTER_CONTINUE;
        }

        /* unset mask of the lock keys */
        mods = xev->xkey.state & ~(_modifiers[7]);

        /* unset mask of Mod4 if using Super_L key as hotkey */
        keysym = XkbKeycodeToKeysym(display, (KeyCode)xev->xkey.keycode, 0, 0);
        if (keysym == GDK_KEY_Super_L) {
                mods = mods & ~((guint)GDK_MOD4_MASK);
        }

        g_hash_table_iter_init(&iter, self->bindings);

        /* Find a matching binding */
        while (g_hash_table_iter_next(&iter, (void **)&key, (void **)&binding)) {
                if (xev->type == KeyPress && xev->xkey.keycode == binding->keycode &&
                    !self->wait_for_release) {
                        /* capture initial key press */
                        if (mods == binding->mods) {
                                self->wait_for_release = TRUE;
                        }
                } else if (xev->type == KeyRelease && xev->xkey.keycode == binding->keycode &&
                           self->wait_for_release) {
                        /* capture release within same shortcut sequence */
                        binding->func(event, binding->udata);
                        self->wait_for_release = FALSE;
                } else if (xev->type == ButtonPress) {
                        /* Modifiers are often used with mouse events.
                         * Don't let the key-binder swallow those */
                        XAllowEvents(display, ReplayPointer, CurrentTime);

                        /* Compiz would rather not have the event sent to it and just read it from
                         * the replayed queue. All other window managers seem to expect an event. */
                        gchar *wm = get_window_manager ();
                        if (g_strcmp0 (wm, "Compiz") != 0) {
                                Window root_return, child_return;
                                int root_x_return, root_y_return;
                                int win_x_return, win_y_return;
                                unsigned int mask_return;
                                XUngrabKeyboard(display, CurrentTime);
                                XUngrabPointer(display, CurrentTime);
                                XQueryPointer(display,
                                              GDK_WINDOW_XID(self->root_window),
                                              &root_return,
                                              &child_return,
                                              &root_x_return,
                                              &root_y_return,
                                              &win_x_return,
                                              &win_y_return,
                                              &mask_return);
                                XSendEvent(display,
                                           child_return,
                                           TRUE,
                                           ButtonPressMask,
                                           xev);
                        }
                        g_free (wm);

                        self->wait_for_release = FALSE;
                } else {
                        XUngrabKeyboard(display, xev->xkey.time);
                        /* when breaking the shortcut sequence, send the event up the window
                         * hierarchy in case it's part of a different shortcut sequence
                         * (e.g. <Mod4>a) */
                        XSendEvent(display,
                                   xev->xkey.window,
                                   TRUE,
                                   KeyPressMask | KeyReleaseMask,
                                   xev);
                        self->wait_for_release = FALSE;
                }
        }

        return GDK_FILTER_CONTINUE;
}

/**
 * Bind a shortcut with the appropriate callback
 */
gboolean brisk_key_binder_bind(BriskKeyBinder *self, const gchar *shortcut, BinderFunc func,
                               gpointer v)
{
        guint keysym;
        GdkModifierType mod;
        Display *display = NULL;
        Window id;
        KeyCode key;
        KeyBinding *bind = NULL;

        if (g_hash_table_contains(self->bindings, shortcut)) {
                return FALSE;
        }

        gtk_accelerator_parse(shortcut, &keysym, &mod);
        display = GDK_WINDOW_XDISPLAY(self->root_window);
        id = GDK_WINDOW_XID(self->root_window);

        key = XKeysymToKeycode(display, keysym);
        if (key == 0) {
                return FALSE;
        }

        bind = g_new0(KeyBinding, 1);
        *bind = (KeyBinding){ .accelerator = shortcut,
                              .keycode = key,
                              .mods = mod,
                              .func = func,
                              .udata = v };

        g_hash_table_insert(self->bindings, g_strdup(shortcut), bind);

        gdk_error_trap_push();
        for (size_t i = 0; i < G_N_ELEMENTS(_modifiers); i++) {
                GdkModifierType m = _modifiers[i];
                XGrabKey(display, key, mod | m, id, TRUE, GrabModeAsync, GrabModeAsync);
        }
        gdk_flush();

        if (!mod) {
                gdk_error_trap_push();
                /* We grab Super+click so that we can forward it to the window manager and allow
                 * Super+click bindings (window move, resize, etc.) */
                XGrabButton(display, AnyButton, Mod4Mask, id, TRUE, ButtonPressMask, GrabModeSync,
                            GrabModeAsync, None, None);
                gdk_flush();
        }

        return TRUE;
}

/**
 * Unbind the shortcut once again, if previously registered
 */
gboolean brisk_key_binder_unbind(BriskKeyBinder *self, const gchar *shortcut)
{
        return g_hash_table_remove(self->bindings, shortcut);
}

/**
 * Handle cleaning up of the keybinding
 */
static void free_keybinding(KeyBinding *binding)
{
        Display *display = NULL;
        Window id;
        GdkWindow *window = NULL;

        window = gdk_get_default_root_window();
        if (window) {
                display = GDK_WINDOW_XDISPLAY(window);
                id = GDK_WINDOW_XID(window);
                XUngrabKey(display, binding->keycode, binding->mods, id);
        } else {
                g_warning("Could not find root window to XUngrabKey");
        }

        g_free(binding);
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
