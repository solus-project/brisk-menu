/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2017-2018 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "all-backend.h"
#include "all-section.h"
#include <glib/gi18n.h>
BRISK_END_PEDANTIC

struct _BriskAllItemsBackendClass {
        BriskBackendClass parent_class;
};

/**
 * BriskAllItemsBackend implements support for .desktop files in Brisk
 */
struct _BriskAllItemsBackend {
        BriskBackend parent;
};

G_DEFINE_TYPE(BriskAllItemsBackend, brisk_all_items_backend, BRISK_TYPE_BACKEND)

static gboolean brisk_all_items_backend_load(BriskBackend *backend);

/**
 * Tell the frontends what we are
 */
static unsigned int brisk_all_items_backend_get_flags(__brisk_unused__ BriskBackend *backend)
{
        return BRISK_BACKEND_SOURCE;
}

static const gchar *brisk_all_items_backend_get_id(__brisk_unused__ BriskBackend *backend)
{
        return "all-items";
}

static const gchar *brisk_all_items_backend_get_display_name(__brisk_unused__ BriskBackend *backend)
{
        return _("All Items");
}

/**
 * brisk_all_items_backend_dispose:
 *
 * Clean up a BriskAllItemsBackend instance
 */
static void brisk_all_items_backend_dispose(GObject *obj)
{
        G_OBJECT_CLASS(brisk_all_items_backend_parent_class)->dispose(obj);
}

/**
 * brisk_all_items_backend_class_init:
 *
 * Handle class initialisation
 */
static void brisk_all_items_backend_class_init(BriskAllItemsBackendClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        BriskBackendClass *b_class = BRISK_BACKEND_CLASS(klazz);

        /* Backend vtable hookup */
        b_class->get_flags = brisk_all_items_backend_get_flags;
        b_class->get_id = brisk_all_items_backend_get_id;
        b_class->get_display_name = brisk_all_items_backend_get_display_name;
        b_class->load = brisk_all_items_backend_load;

        /* gobject vtable hookup */
        obj_class->dispose = brisk_all_items_backend_dispose;
}

/**
 * brisk_all_items_backend_init:
 *
 * Handle construction of the BriskAllItemsBackend
 */
static void brisk_all_items_backend_init(__brisk_unused__ BriskAllItemsBackend *self)
{
}

/**
 * brisk_all_items_backend_load:
 *
 * On load we just emit a new stock section item
 */
static gboolean brisk_all_items_backend_load(BriskBackend *backend)
{
        brisk_backend_section_added(backend, brisk_all_items_section_new());
        return TRUE;
}

/**
 * brisk_all_items_backend_new:
 *
 * Return a newly created BriskAllItemsBackend
 */
BriskBackend *brisk_all_items_backend_new(void)
{
        return g_object_new(BRISK_TYPE_ALL_ITEMS_BACKEND, NULL);
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
