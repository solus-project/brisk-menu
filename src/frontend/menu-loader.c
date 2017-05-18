/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016-2017 Ikey Doherty <ikey@solus-project.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "util.h"

#include <stdlib.h>

BRISK_BEGIN_PEDANTIC
#include "backend/apps/apps-backend.h"
#include "category-button.h"
#include "desktop-button.h"
#include "entry-button.h"
#include "menu-private.h"
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>

BRISK_END_PEDANTIC

static void brisk_menu_window_add_shortcut(BriskMenuWindow *self, const gchar *id);

/**
 * Return the section box for the given backend ID
 */
static GtkWidget *brisk_menu_window_get_section_box(BriskMenuWindow *self, BriskBackend *backend)
{
        return g_hash_table_lookup(self->section_boxes, brisk_backend_get_id(backend));
}

/**
 * Begin a build of the menu structure
 */
static void brisk_menu_window_build(BriskMenuWindow *self)
{
        GtkWidget *sep = NULL;
        autofree(gstrv) *shortcuts = NULL;

        g_message("debug: menu reloaded");

        /* Nuke the current mapping */
        g_hash_table_remove_all(self->item_store);

        brisk_menu_window_set_filters_enabled(self, FALSE);

        /* Clear existing */
        brisk_menu_kill_children(GTK_CONTAINER(self->sidebar));
        brisk_menu_kill_children(GTK_CONTAINER(self->apps));

        /* Special meaning for NULL group */
        self->all_button = brisk_menu_category_button_new(NULL);
        gtk_box_pack_start(GTK_BOX(self->sidebar), self->all_button, FALSE, FALSE, 0);
        gtk_widget_show_all(self->all_button);
        brisk_menu_window_associate_category(self, self->all_button);

        /* TODO: Restore some kind of initial loading ..  */

        /* Separate the things */
        sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_start(GTK_BOX(self->sidebar), sep, FALSE, FALSE, 5);
        gtk_widget_show_all(sep);

        /* Load the shortcuts up */
        shortcuts = g_settings_get_strv(self->settings, "pinned-shortcuts");
        if (!shortcuts) {
                return;
        }

        /* Add from gsettings */
        for (guint i = 0; i < g_strv_length(shortcuts); i++) {
                brisk_menu_window_add_shortcut(self, shortcuts[i]);
        }
        brisk_menu_window_set_filters_enabled(self, TRUE);
}

/**
 * Backend has new items for us, add to the global store
 */
static void brisk_menu_window_add_item(BriskMenuWindow *self, BriskItem *item,
                                       __brisk_unused__ BriskBackend *backend)
{
        GtkWidget *button = NULL;
        const gchar *item_id = brisk_item_get_id(item);

        /* Skip dupes */
        if (g_hash_table_lookup(self->item_store, item_id) != NULL) {
                return;
        }

        g_message("Item: %s", item_id);

        button = brisk_menu_entry_button_new(self->launcher, item);
        gtk_container_add(GTK_CONTAINER(self->apps), button);
        gtk_widget_show_all(button);

        g_hash_table_insert(self->item_store, g_strdup(item_id), button);
}

/**
 * Backend has a new sidebar section for us
 */
static void brisk_menu_window_add_section(BriskMenuWindow *self, BriskSection *section,
                                          __brisk_unused__ BriskBackend *backend)
{
        /* TODO: Add buttons to the section box */
        g_message("Section: %s", brisk_section_get_id(section));
}

/**
 * A backend needs us to purge any data we have for it
 */
static void brisk_menu_window_reset(BriskMenuWindow *self, BriskBackend *backend)
{
        g_message("Backend requests reset: %s", brisk_backend_get_id(backend));
}

/**
 * Load a single backend as stored in the map
 */
static void brisk_menu_window_load_backend(BriskMenuWindow *self, const gchar *backend_id)
{
        BriskBackend *backend = NULL;

        backend = g_hash_table_lookup(self->backends, backend_id);
        if (G_UNLIKELY(backend == NULL)) {
                g_warning("Tried to load invalid backend: '%s'", backend_id);
                return;
        }
        if (!brisk_backend_load(backend)) {
                g_warning("Failed to load backend: '%s'", backend_id);
        }
}

/**
 * Load the menus and place them into the window regions
 */
gboolean brisk_menu_window_load_menus(BriskMenuWindow *self)
{
        g_message("Menu loading not yet implemented!");

        /* TODO: Move this somewhere not fucking stupid. */
        brisk_menu_window_build(self);
        static const gchar *backends[] = {
                "apps",
        };

        /* Now init all backends */
        for (guint i = 0; i < G_N_ELEMENTS(backends); i++) {
                brisk_menu_window_load_backend(self, backends[i]);
        }

        return G_SOURCE_REMOVE;
}

/**
 * Generic function allowing us to set up screen estate and signals for
 * a given backend
 */
static void brisk_menu_window_init_backend(BriskMenuWindow *self, BriskBackend *backend)
{
        GtkWidget *box = NULL;
        const gchar *backend_id = brisk_backend_get_id(backend);

        if (g_hash_table_lookup(self->section_boxes, backend_id)) {
                g_message("debug: tried to load existing backend '%s'", backend_id);
                return;
        }

        /* Hook up the signals first */
        g_signal_connect_swapped(backend,
                                 "item-added",
                                 G_CALLBACK(brisk_menu_window_add_item),
                                 self);
        g_signal_connect_swapped(backend,
                                 "section-added",
                                 G_CALLBACK(brisk_menu_window_add_section),
                                 self);
        g_signal_connect_swapped(backend, "reset", G_CALLBACK(brisk_menu_window_reset), self);

        box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_pack_start(GTK_BOX(self->sidebar), box, FALSE, FALSE, 0);

        /* Always ensure that the box is placed after "All" but not after any
         * non-section boxes
         */
        gtk_box_reorder_child(GTK_BOX(self->sidebar), box, (gint)g_hash_table_size(self->backends));

        g_hash_table_insert(self->section_boxes, (gchar *)backend_id, box);

        g_message("debug: activated backend '%s'", backend_id);
}

/**
 * Utility to insert a single backend
 */
static inline void brisk_menu_window_insert_backend(BriskMenuWindow *self, BriskBackend *backend)
{
        const gchar *backend_id = brisk_backend_get_id(backend);
        g_hash_table_insert(self->backends, (gchar *)backend_id, backend);
        brisk_menu_window_init_backend(self, backend);
}

/**
 * Bring up the initial backends
 *
 * @note Currently we only have AppsBackend
 */
void brisk_menu_window_init_backends(BriskMenuWindow *self)
{
        brisk_menu_window_insert_backend(self, brisk_apps_backend_new());
}

/**
 * brisk_menu_window_add_shortcut
 *
 * If we can create a .desktop launcher for the given name, add a new button to
 * the sidebar as a quick launch facility.
 */
static void brisk_menu_window_add_shortcut(BriskMenuWindow *self, const gchar *id)
{
        GDesktopAppInfo *info = NULL;
        GtkWidget *button = NULL;

        info = g_desktop_app_info_new(id);
        if (!info) {
                g_message("Not adding missing %s to BriskMenu", id);
                return;
        }

        button = brisk_menu_desktop_button_new(self->launcher, G_APP_INFO(info));
        gtk_widget_show_all(button);
        gtk_box_pack_start(GTK_BOX(self->sidebar), button, FALSE, FALSE, 1);
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
