/*
 * This file is part of brisk-menu.
 *
 * Largely inspired by: https://esite.ch/2011/02/global-hotkeys-with-vala/
 *
 * Copyright © 2016 Ikey Doherty <ikey@solus-project.com>
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

        self = BRISK_KEY_BINDER(v);

        if (xev->type != KeyRelease) {
                return GDK_FILTER_CONTINUE;
        }

        /* unset mask of the lock keys */
        mods = xev->xkey.state & ~(_modifiers[7]);

        g_hash_table_iter_init(&iter, self->bindings);

        /* Find a matching binding */
        while (g_hash_table_iter_next(&iter, (void **)&key, (void **)&binding)) {
                if (xev->xkey.keycode == binding->keycode && mods == binding->mods) {
                        binding->func(event, binding->udata);
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
        *bind = (KeyBinding){.accelerator = shortcut,
                             .keycode = key,
                             .mods = mod,
                             .func = func,
                             .udata = v };

        g_hash_table_insert(self->bindings, g_strdup(shortcut), bind);

        gdk_error_trap_push();
        for (size_t i = 0; i < G_N_ELEMENTS(_modifiers); i++) {
                GdkModifierType m = _modifiers[i];
                XGrabKey(display, key, mod | m, id, FALSE, GrabModeAsync, GrabModeAsync);
        }
        gdk_flush();

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
