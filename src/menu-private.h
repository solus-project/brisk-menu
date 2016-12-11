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

#pragma once

#include "menu-window.h"
#include "util.h"
#include <gtk/gtk.h>
#include <matemenu-tree.h>

struct _SolMenuWindowClass {
        GtkWindowClass parent_class;
};

/**
 * SolMenuWindow is the toplevel window type used within the applet.
 */
struct _SolMenuWindow {
        GtkWindow parent;

        /* Categories */
        GtkWidget *sidebar;

        /* Top search entry */
        GtkWidget *search;

        /* Actual applications */
        GtkWidget *apps;

        /* Our tree global reference */
        MateMenuTree *root;

        /* The All categories button */
        GtkWidget *all_button;

        /* The current group used in filtering */
        MateMenuTreeDirectory *active_group;

        /* Search term, may be null at any point. Used for filtering */
        gchar *search_term;
};

/* Split the implementation across multiple files for ease of maintenance */
void sol_menu_window_load_menus(SolMenuWindow *self);
void sol_menu_window_associate_category(SolMenuWindow *self, GtkWidget *button);
void sol_menu_window_search(SolMenuWindow *self, GtkEntry *entry);
gboolean sol_menu_window_filter_apps(GtkListBoxRow *row, gpointer v);

DEF_AUTOFREE(GtkWidget, gtk_widget_destroy)
DEF_AUTOFREE(MateMenuTree, matemenu_tree_unref)
DEF_AUTOFREE(MateMenuTreeDirectory, matemenu_tree_item_unref)
DEF_AUTOFREE(MateMenuTreeItem, matemenu_tree_item_unref)
DEF_AUTOFREE(GSList, g_slist_free)
DEF_AUTOFREE(GList, g_list_free)
DEF_AUTOFREE(GFile, g_object_unref)
DEF_AUTOFREE(GIcon, g_object_unref)

/**
 * Convenience function to remove children from a container
 */
__attribute__((always_inline)) static inline void sol_menu_kill_children(GtkContainer *container)
{
        for (autofree(GList) *elem = gtk_container_get_children(container); elem;
             elem = elem->next) {
                gtk_widget_destroy(GTK_WIDGET(elem->data));
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
