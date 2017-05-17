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
#include "apps-item.h"
#include "apps-section.h"
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <matemenu-tree.h>
BRISK_END_PEDANTIC

DEF_AUTOFREE(GDesktopAppInfo, g_object_unref)

/**
 * Main application menu ID
 */
#define APPS_MENU_ID "mate-applications.menu"

/**
 * Settings menu ID
 */
#define SETTINGS_MENU_ID "matecc.menu"

/**
 * We'll perform reloads 2 seconds after we get the last change
 * notification
 */
#define BRISK_RELOAD_TIME 2000

struct _BriskAppsBackendClass {
        BriskBackendClass parent_class;
};

/**
 * BriskAppsBackend implements support for .desktop files in Brisk
 */
struct _BriskAppsBackend {
        BriskBackend parent;
        GAppInfoMonitor *monitor;
        guint monitor_source_id;
        gboolean loaded;
};

G_DEFINE_TYPE(BriskAppsBackend, brisk_apps_backend, BRISK_TYPE_BACKEND)

static gboolean brisk_apps_backend_load(BriskBackend *backend);
static gboolean brisk_apps_backend_build_from_tree(BriskAppsBackend *self, const gchar *id);
static void brisk_apps_backend_recurse_root(BriskAppsBackend *self,
                                            MateMenuTreeDirectory *directory);
static void brisk_apps_backend_changed(BriskAppsBackend *backend, gpointer v);
static gboolean brisk_apps_backend_reload(BriskAppsBackend *backend, gpointer v);

DEF_AUTOFREE(GSList, g_slist_free)
DEF_AUTOFREE(MateMenuTreeDirectory, matemenu_tree_item_unref)
DEF_AUTOFREE(MateMenuTreeItem, matemenu_tree_item_unref)
DEF_AUTOFREE(MateMenuTree, matemenu_tree_unref)

/**
 * Due to a glib weirdness we must fully invalidate the monitor's cache
 * to force reload events to work again.
 */
static inline void brisk_apps_backend_reset_monitor(void)
{
        g_list_free_full(g_app_info_get_all(), g_object_unref);
}

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

        g_clear_object(&self->monitor);

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
static void brisk_apps_backend_init(BriskAppsBackend *self)
{
        self->monitor = g_app_info_monitor_get();
        g_signal_connect_swapped(self->monitor,
                                 "changed",
                                 G_CALLBACK(brisk_apps_backend_changed),
                                 self);
}

/**
 * brisk_apps_backend_changed:
 *
 * The application availability has now changed due to some on disk alteration.
 * Schedule an update to rebuild the menus
 */
static void brisk_apps_backend_changed(BriskAppsBackend *self, __brisk_unused__ gpointer v)
{
        g_message("debug: menu reload scheduled");
        /* Not interested until we're loaded. */
        if (!self->loaded) {
                return;
        }

        /* Push the change back until the very last event, wait 2 seconds and process it */
        if (self->monitor_source_id > 0) {
                self->monitor_source_id = 0;
                g_source_remove(self->monitor_source_id);
        }

        self->monitor_source_id = g_timeout_add_full(G_PRIORITY_LOW,
                                                     BRISK_RELOAD_TIME,
                                                     (GSourceFunc)brisk_apps_backend_reload,
                                                     self,
                                                     NULL);
}

/**
 * brisk_apps_backend_reload:
 */
static gboolean brisk_apps_backend_reload(BriskAppsBackend *self, __brisk_unused__ gpointer v)
{
        if (self->monitor_source_id < 1) {
                return FALSE;
        }

        /* First things first, reset everything we own */
        brisk_backend_reset(BRISK_BACKEND(self));

        /* Now load them again */
        if (!brisk_apps_backend_build_from_tree(self, APPS_MENU_ID)) {
                g_warning("Failed to load required apps menu id: %s", APPS_MENU_ID);
        }

        if (!brisk_apps_backend_build_from_tree(self, SETTINGS_MENU_ID)) {
                g_warning("Failed to load settings menu id: %s", SETTINGS_MENU_ID);
        }

        /* Reset ourselves for the next time */
        self->monitor_source_id = 0;
        brisk_apps_backend_reset_monitor();
        return FALSE;
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

        /* Unblock monitor */
        self->loaded = TRUE;

        /* Load ourselves a bit later */
        brisk_apps_backend_changed(self, NULL);

        /* Allow the monitor to now work */
        brisk_apps_backend_reset_monitor();

        return TRUE;
}

/**
 * brisk_apps_backend_build_from_tree:
 *
 * Begin building content using the given tree ID, cleaning up once it's done.
 */
static gboolean brisk_apps_backend_build_from_tree(BriskAppsBackend *self, const gchar *menu_id)
{
        autofree(MateMenuTree) *tree = NULL;
        autofree(MateMenuTreeDirectory) *dir = NULL;

        tree = matemenu_tree_lookup(menu_id, MATEMENU_TREE_FLAGS_NONE);
        if (!tree) {
                return FALSE;
        }

        dir = matemenu_tree_get_root_directory(tree);
        if (!dir) {
                return FALSE;
        }
        brisk_apps_backend_recurse_root(self, dir);
        return TRUE;
}

/**
 * brisk_apps_backend_recurse_root:
 *
 * Walk the directory and construct sections/items for every directory/entry
 * that we encounter.
 */
static void brisk_apps_backend_recurse_root(BriskAppsBackend *self,
                                            MateMenuTreeDirectory *directory)
{
        autofree(GSList) *kids = NULL;
        GSList *elem = NULL;

        kids = matemenu_tree_directory_get_contents(directory);

        /* Iterate the root tree */
        for (elem = kids; elem; elem = elem->next) {
                autofree(MateMenuTreeItem) *item = elem->data;

                switch (matemenu_tree_item_get_type(item)) {
                case MATEMENU_TREE_ITEM_DIRECTORY: {
                        MateMenuTreeDirectory *dir = MATEMENU_TREE_DIRECTORY(item);
                        BriskSection *section = NULL;

                        /* If signal subscribers wish to keep it, they can ref it */
                        section = brisk_apps_section_new(dir);
                        brisk_backend_section_added(BRISK_BACKEND(self), section);
                        g_object_unref(section);

                        /* Descend into the section */
                        brisk_apps_backend_recurse_root(self, dir);
                } break;
                case MATEMENU_TREE_ITEM_ENTRY: {
                        /* TODO: Emit a real item here */
                        MateMenuTreeEntry *entry = MATEMENU_TREE_ENTRY(item);
                        autofree(GDesktopAppInfo) *info = NULL;
                        const gchar *desktop_file = NULL;
                        BriskItem *app_item = NULL;

                        desktop_file = matemenu_tree_entry_get_desktop_file_path(entry);

                        /* idk */
                        if (!desktop_file) {
                                break;
                        }

                        /* Must have a desktop file */
                        info = g_desktop_app_info_new_from_filename(desktop_file);
                        if (!info) {
                                break;
                        }
                        /* If signal subscribers wish to keep it, they can ref it */
                        app_item = brisk_apps_item_new(info);
                        brisk_backend_item_added(BRISK_BACKEND(self), app_item);
                        g_object_unref(app_item);
                } break;
                default:
                        break;
                }
        }
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
