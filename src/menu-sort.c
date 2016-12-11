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

#include "util.h"

#include <stdlib.h>

BRISK_BEGIN_PEDANTIC
#include "entry-button.h"
#include "menu-private.h"
#include <matemenu-tree.h>
#include <string.h>
BRISK_END_PEDANTIC

/**
 * Compute a score for the given entry based on the input term.
 */
static gint brisk_get_entry_score(MateMenuTreeEntry *entry, gchar *term)
{
        gint score = 0;
        autofree(gchar) *name = NULL;
        char *find = NULL;

        name = g_ascii_strdown(matemenu_tree_entry_get_name(entry), -1);
        if (g_str_equal(name, term)) {
                score += 100;
        } else if (g_str_has_prefix(name, term)) {
                score += 50;
        }

        find = strstr(name, term);
        if (find) {
                score += 20 + (int)strlen(find);
        }

        score += strcmp(name, term);

        return score;
}

gint brisk_menu_window_sort(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer v)
{
        GtkWidget *child1, *child2 = NULL;
        MateMenuTreeEntry *entryA, *entryB = NULL;
        const gchar *nameA, *nameB = NULL;
        BriskMenuWindow *self = NULL;

        self = BRISK_MENU_WINDOW(v);

        child1 = gtk_bin_get_child(GTK_BIN(row1));
        child2 = gtk_bin_get_child(GTK_BIN(row2));

        g_object_get(child1, "entry", &entryA, NULL);
        g_object_get(child2, "entry", &entryB, NULL);

        if (self->search_term) {
                gint sc1 = brisk_get_entry_score(entryA, self->search_term);
                gint sc2 = brisk_get_entry_score(entryB, self->search_term);
                return (sc1 > sc2) - (sc1 - sc2);
        }

        nameA = matemenu_tree_entry_get_display_name(entryA);
        nameB = matemenu_tree_entry_get_display_name(entryB);

        /* TODO: Get score for the search term. */
        return g_strcmp0(nameA, nameB);
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
