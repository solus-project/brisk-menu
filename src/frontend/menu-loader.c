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
#include "entry-button.h"
#include "menu-private.h"
#include <gtk/gtk.h>

BRISK_END_PEDANTIC

/**
 * Return the section box for the given backend ID
 */
static GtkWidget *brisk_menu_window_get_section_box(BriskMenuWindow *self, BriskBackend *backend)
{
        return g_hash_table_lookup(self->section_boxes, brisk_backend_get_id(backend));
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
        GtkWidget *button = NULL;
        const gchar *section_id = brisk_section_get_id(section);
        GtkWidget *box_target = NULL;

        /* Skip dupes. Sections are uniquely namespaced */
        if (g_hash_table_lookup(self->item_store, section_id) != NULL) {
                return;
        }

        box_target = brisk_menu_window_get_section_box(self, backend);

        button = brisk_menu_category_button_new(section);
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(button), GTK_RADIO_BUTTON(self->all_button));
        gtk_box_pack_start(GTK_BOX(box_target), button, FALSE, FALSE, 0);
        brisk_menu_window_associate_category(self, button);
        gtk_widget_show_all(button);

        /* Avoid new dupes */
        g_hash_table_insert(self->item_store, g_strdup(section_id), button);
}

/**
 * Handle deletion for children in the sidebar
 */
static void brisk_menu_window_remove_category(GtkWidget *widget, BriskMenuWindow *self)
{
        BriskSection *section = NULL;
        const gchar *section_id = NULL;

        if (!BRISK_IS_MENU_CATEGORY_BUTTON(widget)) {
                return;
        }

        g_object_get(widget, "section", &section, NULL);
        if (!section) {
                g_warning("missing section for category button");
        }

        section_id = brisk_section_get_id(section);

        g_hash_table_remove(self->item_store, section_id);
        gtk_widget_destroy(widget);
}

/**
 * A backend needs us to purge any data we have for it
 */
static void brisk_menu_window_reset(BriskMenuWindow *self, BriskBackend *backend)
{
        GtkWidget *box_target = NULL;
        GList *kids = NULL, *elem = NULL;
        const gchar *backend_id = NULL;

        backend_id = brisk_backend_get_id(backend);

        box_target = brisk_menu_window_get_section_box(self, backend);
        gtk_container_foreach(GTK_CONTAINER(box_target),
                              (GtkCallback)brisk_menu_window_remove_category,
                              self);

        /* Manual work for the items */
        kids = gtk_container_get_children(GTK_CONTAINER(self->apps));
        for (elem = kids; elem; elem = elem->next) {
                GtkWidget *row = elem->data;
                GtkWidget *child = NULL;
                BriskItem *item = NULL;
                const gchar *local_backend_id = NULL;
                const gchar *local_id = NULL;

                if (!GTK_IS_BIN(GTK_BIN(row))) {
                        continue;
                }

                child = gtk_bin_get_child(GTK_BIN(row));
                if (!BRISK_IS_MENU_ENTRY_BUTTON(child)) {
                        continue;
                }

                g_object_get(child, "item", &item, NULL);
                if (!item) {
                        g_warning("missing item for entry in backend '%s'", backend_id);
                        continue;
                }

                local_backend_id = brisk_item_get_backend_id(item);
                if (!g_str_equal(backend_id, local_backend_id)) {
                        continue;
                }
                local_id = brisk_item_get_id(item);
                g_hash_table_remove(self->item_store, local_id);
                gtk_widget_destroy(row);
        }
        g_list_free(kids);
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
