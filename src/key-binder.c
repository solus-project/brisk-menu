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
#include <gdk/gdk.h>
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
};

static GdkFilterReturn brisk_key_binder_filter(GdkXEvent *xevent, GdkEvent *event, gpointer v);

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

        self = BRISK_KEY_BINDER(v);

        return GDK_FILTER_CONTINUE;
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
