/*
 * This file is part of brisk-menu.
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
BRISK_END_PEDANTIC

struct _BriskKeyBinderClass {
        GObjectClass parent_class;
};

/**
 * BriskKeyBinder is used to bind global x11 shortcuts
 */
struct _BriskKeyBinder {
        GObject parent;
};

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
        /* NOOP */
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
