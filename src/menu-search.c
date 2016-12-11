/*
 * This file is part of mate-solmenu.
 *
 * Copyright © 2016 Ikey Doherty <ikey@solus-proejct.com>
 *
 * mate-solmenu is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#include "util.h"

#include <stdlib.h>

SOLUS_BEGIN_PEDANTIC
#include "menu-private.h"
#include <gtk/gtk.h>
#include <string.h>
SOLUS_END_PEDANTIC

/**
 * sol_menu_window_search:
 *
 * Callback for the text entry changing. Set the search term and force
 * an invalidation of our filters.
 */
void sol_menu_window_search(SolMenuWindow *self, GtkEntry *entry)
{
        const gchar *search_term = NULL;

        /* Remove old search term */
        search_term = gtk_entry_get_text(entry);
        g_clear_pointer(&self->search_term, g_free);

        /* New search term, always lower case for simplicity */
        self->search_term = g_strstrip(g_ascii_strdown(search_term, -1));

        /* Reset our search term if it's not valid anymore, or whitespace */
        if (strlen(self->search_term) > 0) {
                gtk_widget_set_sensitive(self->sidebar, FALSE);
        } else {
                gtk_widget_set_sensitive(self->sidebar, TRUE);
                g_clear_pointer(&self->search_term, g_free);
        }

        /* Now filter again */
        gtk_list_box_invalidate_filter(GTK_LIST_BOX(self->apps));
}

/**
 * sol_menu_window_filter_apps:
 *
 * Responsible for filtering the selection based on active group or search
 * term.
 */
gboolean sol_menu_window_filter_apps(GtkListBoxRow *row, gpointer v)
{
        SolMenuWindow *self = NULL;
        MateMenuTreeEntry *entry = NULL;
        GtkWidget *child = NULL;
        MateMenuTreeDirectory *parent = NULL;

        self = SOL_MENU_WINDOW(v);

        /* All visible */
        if (!self->active_group) {
                return TRUE;
        }

        /* Grab our Entry widget */
        child = gtk_bin_get_child(GTK_BIN(row));

        g_object_get(child, "entry", &entry, NULL);
        if (!entry) {
                return FALSE;
        }

        parent = matemenu_tree_item_get_parent(MATEMENU_TREE_ITEM(entry));

        /* Check if it matches the current group */
        if (self->active_group != parent) {
                return FALSE;
        }
        return TRUE;
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
