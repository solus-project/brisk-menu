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
 * Recurse the given directory and any of it's children directories. Add all of
 * the directories to the sidebar, and then (TODO) stick "normal" types into the
 * content section.
 */
static void sol_menu_window_recurse_root(SolMenuWindow *self, MateMenuTreeDirectory *directory)
{
        autofree(GSList) *kids = NULL;
        GSList *elem = NULL;

        kids = matemenu_tree_directory_get_contents(directory);

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
                        gtk_widget_show_all(button);

                        sol_menu_window_recurse_root(self, dir);
                } break;
                case MATEMENU_TREE_ITEM_ENTRY: {
                        GtkWidget *button = NULL;
                        MateMenuTreeEntry *entry = MATEMENU_TREE_ENTRY(item);
                        const gchar *lab = matemenu_tree_entry_get_display_name(entry);

                        button = gtk_button_new_with_label(lab);
                        gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
                        gtk_widget_set_can_focus(button, FALSE);
                        gtk_container_add(GTK_CONTAINER(self->apps), button);
                        gtk_widget_show_all(button);
                } break;
                default:
                        break;
                }
        }
}

/**
 * Begin a build of the menu structure
 */
static void sol_menu_window_build(SolMenuWindow *self)
{
        MateMenuTreeDirectory *dir = NULL;

        g_message("debug: menu reload");

        dir = matemenu_tree_get_root_directory(self->root);

        /* Clear existing */
        sol_menu_kill_children(GTK_CONTAINER(self->sidebar));
        sol_menu_kill_children(GTK_CONTAINER(self->apps));

        /* Populate with new */
        sol_menu_window_recurse_root(self, dir);
}

/**
 * Allow us to load the menu on the idle loop
 */
static inline gboolean inline_reload_menu(SolMenuWindow *self)
{
        sol_menu_window_build(self);
        return FALSE;
}

/**
 * Handle rebuilding of tree in response to a change.
 */
static inline void sol_menu_window_reloaded(__solus_unused__ MateMenuTree *tree, gpointer v)
{
        g_idle_add((GSourceFunc)inline_reload_menu, v);
}

/**
 * Load the menus and place them into the window regions
 */
void sol_menu_window_load_menus(SolMenuWindow *self)
{
        MateMenuTree *tree = NULL;

        /* Load menu */
        tree = matemenu_tree_lookup("mate-applications.menu", MATEMENU_TREE_FLAGS_NONE);
        matemenu_tree_add_monitor(tree, sol_menu_window_reloaded, self);
        self->root = tree;

        /* Load menus on idle */
        g_idle_add((GSourceFunc)inline_reload_menu, self);
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
