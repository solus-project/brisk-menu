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

#pragma once

#include "launcher.h"
#include "libsaver-glue.h"
#include "libsession-glue.h"
#include "menu-window.h"
#include "util.h"
#include <gtk/gtk.h>
#include <matemenu-tree.h>

struct _BriskMenuWindowClass {
        GtkWindowClass parent_class;
};

/**
 * BriskMenuWindow is the toplevel window type used within the applet.
 */
struct _BriskMenuWindow {
        GtkWindow parent;

        /* Control launches */
        BriskMenuLauncher *launcher;

        /* Categories */
        GtkWidget *sidebar;
        GtkWidget *sidebar_scroll;

        /* VBox for the sidebar container */
        GtkWidget *sidebar_wrap;

        /* Top search entry */
        GtkWidget *search;

        /* Actual applications */
        GtkWidget *apps;
        GtkWidget *apps_scroll;

        /* Our tree global reference */
        MateMenuTree *root;

        /* The All categories button */
        GtkWidget *all_button;

        /* The current group used in filtering */
        MateMenuTreeDirectory *active_group;

        /* The current tree parent used in filtering */
        MateMenuTree *active_tree;

        /* Search term, may be null at any point. Used for filtering */
        gchar *search_term;

        /* Our CSS assets */
        GtkCssProvider *css;

        gboolean filtering;

        /* Whether we have a grab or not */
        gboolean grabbed;

        /* Session management */
        GnomeSessionManager *session;
        GnomeScreenSaver *saver;
        GtkWidget *button_lock;
        GtkWidget *button_logout;
        GtkWidget *button_shutdown;
};

/* Split the implementation across multiple files for ease of maintenance */
gboolean brisk_menu_window_load_menus(BriskMenuWindow *self);
void brisk_menu_window_associate_category(BriskMenuWindow *self, GtkWidget *button);

/* Search */
void brisk_menu_window_search(BriskMenuWindow *self, GtkEntry *entry);
gboolean brisk_menu_window_filter_apps(GtkListBoxRow *row, gpointer v);
void brisk_menu_window_activated(BriskMenuWindow *self, GtkListBoxRow *row, gpointer v);
void brisk_menu_window_clear_search(GtkEntry *entry, GtkEntryIconPosition pos, GdkEvent *event,
                                    gpointer v);

/* Sorting */
gint brisk_menu_window_sort(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer v);

/* Keyboard */
gboolean brisk_menu_window_key_release(BriskMenuWindow *self, GdkEvent *event, gpointer v);
void brisk_menu_window_key_activate(BriskMenuWindow *self, gpointer v);

void brisk_menu_window_set_filters_enabled(BriskMenuWindow *self, gboolean enabled);

/* Global grabs */
void brisk_menu_window_configure_grabs(BriskMenuWindow *self);

/* Session controls */
void brisk_menu_window_setup_session(BriskMenuWindow *self);

DEF_AUTOFREE(GtkWidget, gtk_widget_destroy)
DEF_AUTOFREE(MateMenuTree, matemenu_tree_unref)
DEF_AUTOFREE(MateMenuTreeDirectory, matemenu_tree_item_unref)
DEF_AUTOFREE(MateMenuTreeItem, matemenu_tree_item_unref)
DEF_AUTOFREE(GSList, g_slist_free)
DEF_AUTOFREE(GList, g_list_free)
DEF_AUTOFREE(GFile, g_object_unref)
DEF_AUTOFREE(GIcon, g_object_unref)
DEF_AUTOFREE(gchar, g_free)
DEF_AUTOFREE(GdkAppLaunchContext, g_object_unref)
DEF_AUTOFREE(GError, g_error_free)

/**
 * Convenience function to remove children from a container
 */
__attribute__((always_inline)) static inline void brisk_menu_kill_children(GtkContainer *container)
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
