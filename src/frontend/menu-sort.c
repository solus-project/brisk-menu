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
#include "entry-button.h"
#include "menu-private.h"
#include <string.h>
BRISK_END_PEDANTIC

/**
 * Compute a score for the given entry based on the input term.
 */
__brisk_pure__ static gint brisk_get_entry_score(BriskItem *item, const gchar *term)
{
        gint score = 0;
        autofree(gchar) *name = NULL;
        char *find = NULL;

        name = g_ascii_strdown(brisk_item_get_name(item), -1);
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

__brisk_pure__ gint brisk_menu_window_sort(BriskMenuWindow *self, BriskItem *itemA,
                                           BriskItem *itemB)
{
        autofree(gchar) *nameA = NULL;
        autofree(gchar) *nameB = NULL;
        gint sc1 = -1, sc2 = -1;

        /* Handle normal searching */
        if (self->search_term) {
                sc1 = brisk_get_entry_score(itemA, self->search_term);
                sc2 = brisk_get_entry_score(itemB, self->search_term);
                return (sc1 > sc2) - (sc1 - sc2);
        }

        if (!self->active_section) {
                goto basic_sort;
        }

        sc1 = brisk_section_get_sort_order(self->active_section, itemB);
        sc2 = brisk_section_get_sort_order(self->active_section, itemA);

        /* Negative score means the section doesn't support custom ordering */
        if (sc1 >= 0 || sc2 >= 0) {
                /* Sort based on the sections understanding */
                return (sc1 > sc2) - (sc1 - sc2);
        }

basic_sort:
        /* Ensure we compare lower case only */
        nameA = g_ascii_strdown(brisk_item_get_display_name(itemA), -1);
        nameB = g_ascii_strdown(brisk_item_get_display_name(itemB), -1);

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
