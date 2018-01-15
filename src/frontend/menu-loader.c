/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016-2018 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "util.h"

#include <stdlib.h>

BRISK_BEGIN_PEDANTIC
#include "backend/all-items/all-backend.h"
#include "backend/apps/apps-backend.h"
#include "backend/favourites/favourites-backend.h"
#include "entry-button.h"
#include "menu-private.h"
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

/**
 * Return the section box for the given backend ID
 */
GtkWidget *brisk_menu_window_get_section_box(BriskMenuWindow *self, BriskBackend *backend)
{
        return g_hash_table_lookup(self->section_boxes, brisk_backend_get_id(backend));
}

/**
 * A backend needs us to hide the window
 */
static void brisk_menu_window_hide(BriskMenuWindow *self, __brisk_unused__ BriskBackend *backend)
{
        gtk_widget_hide(GTK_WIDGET(self));
}

/**
 * Load the menus and place them into the window regions
 */
gboolean brisk_menu_window_load_menus(BriskMenuWindow *self)
{
        GHashTableIter iter;
        gchar *backend_id = NULL;
        BriskBackend *backend = NULL;

        /* We only call load() on backends exposing BRISK_BACKEND_SOURCE */
        g_hash_table_iter_init(&iter, self->backends);
        while (g_hash_table_iter_next(&iter, (void **)&backend_id, (void **)&backend)) {
                guint flags = brisk_backend_get_flags(backend);
                if ((flags & BRISK_BACKEND_SOURCE) != BRISK_BACKEND_SOURCE) {
                        continue;
                }
                if (!brisk_backend_load(backend)) {
                        g_warning("Failed to load source backend: '%s'", backend_id);
                }
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
        g_signal_connect_swapped(backend,
                                 "invalidate-filter",
                                 G_CALLBACK(brisk_menu_window_invalidate_filter),
                                 self);
        g_signal_connect_swapped(backend, "hide-menu", G_CALLBACK(brisk_menu_window_hide), self);
        g_signal_connect_swapped(backend, "reset", G_CALLBACK(brisk_menu_window_reset), self);

        box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_pack_start(GTK_BOX(self->section_box_holder), box, FALSE, FALSE, 0);

        /* Always ensure that the box is placed after "All" but not after any
         * non-section boxes
         */
        gtk_box_reorder_child(GTK_BOX(self->section_box_holder),
                              box,
                              (gint)g_hash_table_size(self->backends));

        g_hash_table_insert(self->section_boxes, (gchar *)backend_id, box);
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
 * Handle deletion for children in the sidebar
 */
void brisk_menu_window_remove_category(GtkWidget *widget, BriskMenuWindow *self)
{
        BriskSection *section = NULL;
        const gchar *section_id = NULL;

        if (!GTK_IS_RADIO_BUTTON(widget)) {
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
 * Bring up the initial backends
 */
void brisk_menu_window_init_backends(BriskMenuWindow *self)
{
        brisk_menu_window_insert_backend(self, brisk_all_items_backend_new());
        brisk_menu_window_insert_backend(self, brisk_favourites_backend_new());
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
