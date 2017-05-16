/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2017 Ikey Doherty <ikey@solus-project.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "backend.h"
BRISK_END_PEDANTIC

G_DEFINE_TYPE(BriskBackend, brisk_backend, G_TYPE_OBJECT)

/**
 * brisk_backend_dispose:
 *
 * Clean up a BriskBackend instance
 */
static void brisk_backend_dispose(GObject *obj)
{
        G_OBJECT_CLASS(brisk_backend_parent_class)->dispose(obj);
}

/**
 * brisk_backend_class_init:
 *
 * Handle class initialisation
 */
static void brisk_backend_class_init(BriskBackendClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_backend_dispose;
}

/**
 * brisk_backend_init:
 *
 * Handle construction of the BriskBackend
 */
static void brisk_backend_init(__brisk_unused__ BriskBackend *self)
{
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
