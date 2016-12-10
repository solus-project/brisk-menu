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
SOLUS_END_PEDANTIC

/**
 * Load the menus and place them into the window regions
 */
void sol_menu_window_load_menus(SolMenuWindow *self)
{
        autofree(MateMenuTree) *tree = NULL;
        autofree(GSList) *kids = NULL;
        GSList *elem = NULL;
        MateMenuTreeDirectory *dir = NULL;

        tree = matemenu_tree_lookup("mate-applications.menu", MATEMENU_TREE_FLAGS_NONE);

        dir = matemenu_tree_get_root_directory(tree);
        kids = matemenu_tree_directory_get_contents(dir);

        sol_menu_kill_children(GTK_CONTAINER(self->sidebar));

        /* Iterate the root tree
         * TODO: Traverse it _properly_..
         */
        for (elem = kids; elem; elem = elem->next) {
                MateMenuTreeItem *item = elem->data;

                /* DEMO Bits - this whole function needs to move down and become a
                 * recursive function. */
                switch (matemenu_tree_item_get_type(item)) {
                case MATEMENU_TREE_ITEM_DIRECTORY: {
                        GtkWidget *button = NULL;
                        MateMenuTreeDirectory *dir = MATEMENU_TREE_DIRECTORY(item);
                        const gchar *lab = matemenu_tree_directory_get_name(dir);

                        button = gtk_button_new_with_label(lab);
                        gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
                        gtk_widget_set_can_focus(button, FALSE);
                        gtk_container_add(GTK_CONTAINER(self->sidebar), button);
                } break;
                default:
                        break;
                }
        }
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
