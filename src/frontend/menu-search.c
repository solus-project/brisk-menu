/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016-2017 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "util.h"

#include <stdlib.h>

BRISK_BEGIN_PEDANTIC
#include "menu-private.h"
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>
#include <string.h>
BRISK_END_PEDANTIC

/**
 * Update sensitivity - ignoring non checkboxes
 */
static void brisk_menu_set_checks_sensitive(GtkWidget *parent, gboolean sensitive)
{
        autofree(GList) *kids = NULL;
        GList *elem = NULL;

        kids = gtk_container_get_children(GTK_CONTAINER(parent));
        for (elem = kids; elem; elem = elem->next) {
                GtkWidget *kid = elem->data;
                if (!GTK_IS_CHECK_BUTTON(kid)) {
                        continue;
                }
                gtk_widget_set_sensitive(kid, sensitive);
        }
}

/**
 * Ask all boxes to update their section children
 */
static void brisk_menu_set_categories_sensitive(BriskMenuWindow *self, gboolean sensitive)
{
        brisk_menu_set_checks_sensitive(self->sidebar, sensitive);
        GHashTableIter iter;
        __attribute__((unused)) gchar *key = NULL;
        GtkWidget *box = NULL;

        /* Update all sideboxes for search */
        g_hash_table_iter_init(&iter, self->section_boxes);
        while (g_hash_table_iter_next(&iter, (void **)&key, (void **)&box)) {
                brisk_menu_set_checks_sensitive(box, sensitive);
        }
}

/**
 * brisk_menu_window_clear_search:
 *
 * Simply put, resets the active search term
 */
void brisk_menu_window_clear_search(GtkEntry *entry, GtkEntryIconPosition pos,
                                    __brisk_unused__ GdkEvent *event, __brisk_unused__ gpointer v)
{
        if (pos != GTK_ENTRY_ICON_SECONDARY) {
                return;
        }
        gtk_entry_set_text(entry, "");
}

/**
 * brisk_menu_window_search:
 *
 * Callback for the text entry changing. Set the search term and force
 * an invalidation of our filters.
 */
void brisk_menu_window_search(BriskMenuWindow *self, GtkEntry *entry)
{
        const gchar *search_term = NULL;

        if (!self->filtering) {
                return;
        }

        /* Remove old search term */
        search_term = gtk_entry_get_text(entry);
        g_clear_pointer(&self->search_term, g_free);

        /* New search term, always lower case for simplicity */
        self->search_term = g_strstrip(g_ascii_strdown(search_term, -1));

        /* Reset our search term if it's not valid anymore, or whitespace */
        if (strlen(self->search_term) > 0) {
                brisk_menu_set_categories_sensitive(self, FALSE);
        } else {
                brisk_menu_set_categories_sensitive(self, TRUE);
                g_clear_pointer(&self->search_term, g_free);
        }

        /* Now filter again */
        gtk_list_box_invalidate_filter(GTK_LIST_BOX(self->apps));
        gtk_list_box_invalidate_sort(GTK_LIST_BOX(self->apps));
}

/**
 * brisk_menu_window_filter_section:
 *
 * This function will handle filtering the selection based on the active
 * section, when no search term is applied.
 *
 * Returning TRUE means the item should be displayed
 */
__brisk_pure__ static gboolean brisk_menu_window_filter_section(BriskMenuWindow *self,
                                                                BriskItem *item)
{
        /* All visible */
        if (!self->active_section) {
                return TRUE;
        }

        return brisk_section_can_show_item(self->active_section, item);
}

/**
 * brisk_menu_window_filter_apps:
 *
 * Responsible for filtering the selection based on active group or search
 * term.
 */
__brisk_pure__ gboolean brisk_menu_window_filter_apps(GtkListBoxRow *row, gpointer v)
{
        BriskMenuWindow *self = NULL;
        BriskItem *item = NULL;
        GtkWidget *child = NULL;
        const gchar *item_id = NULL;
        GtkWidget *compare_child = NULL;

        self = BRISK_MENU_WINDOW(v);

        if (!self->filtering) {
                return FALSE;
        }

        /* Grab our Entry widget */
        child = gtk_bin_get_child(GTK_BIN(row));

        g_object_get(child, "item", &item, NULL);
        if (!item) {
                return FALSE;
        }

        /* Item ID's are unique, so the last entry added for an ID is the
         * button we want to display. Basically, a button can be duplicated and
         * appear in multiple categories. By keeping a unique ID -> button mapping,
         * we ensure we only ever show it once in the search function.
         */
        item_id = brisk_item_get_id(item);
        if (item_id) {
                compare_child = g_hash_table_lookup(self->item_store, item_id);
                if (compare_child && compare_child != child) {
                        return FALSE;
                }
        }

        /* If we have no search term, filter on the section */
        if (!self->search_term) {
                return brisk_menu_window_filter_section(self, item);
        }

        /* Have search term? Filter on that. */
        return brisk_item_matches_search(item, self->search_term);
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
