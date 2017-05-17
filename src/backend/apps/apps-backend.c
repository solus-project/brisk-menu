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
#include "apps-backend.h"
#include <glib/gi18n.h>
#include <matemenu-tree.h>
BRISK_END_PEDANTIC

/**
 * Main application menu ID
 */
#define APPS_MENU_ID "mate-applications.menu"

/**
 * Settings menu ID
 */
#define SETTINGS_MENU_ID "matecc.menu"

struct _BriskAppsBackendClass {
        BriskBackend parent_class;
};

/**
 * BriskAppsBackend implements support for .desktop files in Brisk
 */
struct _BriskAppsBackend {
        BriskBackend parent;
        MateMenuTree *tree_apps;
        MateMenuTree *tree_settings;
};

G_DEFINE_TYPE(BriskAppsBackend, brisk_apps_backend, BRISK_TYPE_BACKEND)

static gboolean brisk_apps_backend_load(BriskBackend *backend);

/**
 * Tell the frontends what we are
 */
static unsigned int brisk_apps_backend_get_flags(__brisk_unused__ BriskBackend *backend)
{
        return BRISK_BACKEND_SOURCE;
}

static const gchar *brisk_apps_backend_get_id(__brisk_unused__ BriskBackend *backend)
{
        return "apps";
}

static const gchar *brisk_apps_backend_get_display_name(__brisk_unused__ BriskBackend *backend)
{
        return _("Applications");
}

/**
 * brisk_apps_backend_dispose:
 *
 * Clean up a BriskAppsBackend instance
 */
static void brisk_apps_backend_dispose(GObject *obj)
{
        BriskAppsBackend *self = BRISK_APPS_BACKEND(obj);

        g_clear_pointer(&self->tree_apps, matemenu_tree_unref);
        g_clear_pointer(&self->tree_settings, matemenu_tree_unref);

        G_OBJECT_CLASS(brisk_apps_backend_parent_class)->dispose(obj);
}

/**
 * brisk_apps_backend_class_init:
 *
 * Handle class initialisation
 */
static void brisk_apps_backend_class_init(BriskAppsBackendClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        BriskBackendClass *b_class = BRISK_BACKEND_CLASS(klazz);

        /* Backend vtable hookup */
        b_class->get_flags = brisk_apps_backend_get_flags;
        b_class->get_id = brisk_apps_backend_get_id;
        b_class->get_display_name = brisk_apps_backend_get_display_name;
        b_class->load = brisk_apps_backend_load;

        /* gobject vtable hookup */
        obj_class->dispose = brisk_apps_backend_dispose;
}

/**
 * brisk_apps_backend_init:
 *
 * Handle construction of the BriskAppsBackend
 */
static void brisk_apps_backend_init(__brisk_unused__ BriskAppsBackend *self)
{
}

/**
 * brisk_apps_backend_load:
 *
 * Begin loading menu structures and fire off the first scheduled adds to the
 * menu.
 */
static gboolean brisk_apps_backend_load(BriskBackend *backend)
{
        BriskAppsBackend *self = NULL;

        self = BRISK_APPS_BACKEND(backend);
        g_clear_pointer(&self->tree_apps, matemenu_tree_unref);
        g_clear_pointer(&self->tree_settings, matemenu_tree_unref);

        self->tree_apps = matemenu_tree_lookup(APPS_MENU_ID, MATEMENU_TREE_FLAGS_NONE);

        if (!self->tree_apps) {
                g_warning("Failed to load any applications");
                return FALSE;
        }

        /* TODO: Stick in the idle monitor here */

        self->tree_settings = matemenu_tree_lookup(SETTINGS_MENU_ID, MATEMENU_TREE_FLAGS_NONE);
        if (!self->tree_settings) {
                g_warning("Cannot load MATE Control Center items from %s", SETTINGS_MENU_ID);
                return TRUE;
        }
        /* TODO: Stick monitor here too */

        /* Basically done */
        return TRUE;
}

/**
 * brisk_apps_backend_new:
 *
 * Return a newly created BriskAppsBackend
 */
BriskBackend *brisk_apps_backend_new(void)
{
        return g_object_new(BRISK_TYPE_APPS_BACKEND, NULL);
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
