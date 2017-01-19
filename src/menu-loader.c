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
#include "category-button.h"
#include "desktop-button.h"
#include "entry-button.h"
#include "menu-private.h"
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>

BRISK_END_PEDANTIC

static void brisk_menu_window_add_shortcut(BriskMenuWindow *self, const gchar *id);

static guint source_id = 0;

#define BRISK_RELOAD_TIME 1500

/**
 * Recurse the given directory and any of it's children directories. Add all of
 * the directories to the sidebar, and then (TODO) stick "normal" types into the
 * content section.
 */
static void brisk_menu_window_recurse_root(BriskMenuWindow *self, MateMenuTreeDirectory *directory)
{
        autofree(GSList) *kids = NULL;
        GSList *elem = NULL;
        MateMenuTree *root_tree = NULL;

        kids = matemenu_tree_directory_get_contents(directory);
        root_tree = matemenu_tree_directory_get_tree(directory);

        /* Iterate the root tree */
        for (elem = kids; elem; elem = elem->next) {
                autofree(MateMenuTreeItem) *item = elem->data;

                switch (matemenu_tree_item_get_type(item)) {
                case MATEMENU_TREE_ITEM_DIRECTORY: {
                        /* Tree directory maps to a BriskMenuCategoryButton */
                        GtkWidget *button = NULL;
                        MateMenuTreeDirectory *dir = MATEMENU_TREE_DIRECTORY(item);

                        button = brisk_menu_category_button_new(root_tree, dir);
                        gtk_radio_button_join_group(GTK_RADIO_BUTTON(button),
                                                    GTK_RADIO_BUTTON(self->all_button));
                        gtk_box_pack_start(GTK_BOX(self->sidebar), button, FALSE, FALSE, 0);
                        brisk_menu_window_associate_category(self, button);
                        gtk_widget_show_all(button);

                        brisk_menu_window_recurse_root(self, dir);
                } break;
                case MATEMENU_TREE_ITEM_ENTRY: {
                        /* Tree entry maps to a BriskMenuEntryButton */
                        GtkWidget *button = NULL;
                        MateMenuTreeEntry *entry = MATEMENU_TREE_ENTRY(item);
                        const gchar *desktop_id = NULL;

                        button = brisk_menu_entry_button_new(self->launcher, root_tree, entry);
                        gtk_container_add(GTK_CONTAINER(self->apps), button);
                        gtk_widget_show_all(button);

                        /* Store the .id -> button so we can filter on the last included .id */
                        desktop_id = matemenu_tree_entry_get_desktop_file_id(entry);
                        if (!desktop_id) {
                                break;
                        }
                        g_hash_table_insert(self->desktop_store, g_strdup(desktop_id), button);
                } break;
                default:
                        break;
                }
        }
}

/**
 * Begin a build of the menu structure
 */
static void brisk_menu_window_build(BriskMenuWindow *self)
{
        autofree(MateMenuTreeDirectory) *dir = NULL;
        GtkWidget *sep = NULL;
        autofree(gstrv) *shortcuts = NULL;

        g_message("debug: menu reloaded");

        /* Nuke the current mapping */
        g_hash_table_remove_all(self->desktop_store);

        brisk_menu_window_set_filters_enabled(self, FALSE);

        dir = matemenu_tree_get_root_directory(self->root);

        /* Clear existing */
        brisk_menu_kill_children(GTK_CONTAINER(self->sidebar));
        brisk_menu_kill_children(GTK_CONTAINER(self->apps));

        /* Special meaning for NULL group */
        self->all_button = brisk_menu_category_button_new(NULL, NULL);
        gtk_box_pack_start(GTK_BOX(self->sidebar), self->all_button, FALSE, FALSE, 0);
        gtk_widget_show_all(self->all_button);
        brisk_menu_window_associate_category(self, self->all_button);

        /* Populate with new */
        brisk_menu_window_recurse_root(self, dir);

        /* Separate the things */
        sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_start(GTK_BOX(self->sidebar), sep, FALSE, FALSE, 5);
        gtk_widget_show_all(sep);

        /* Load the shortcuts up */
        shortcuts = g_settings_get_strv(self->settings, "pinned-shortcuts");
        if (!shortcuts) {
                return;
        }

        /* Add from gsettings */
        for (guint i = 0; i < g_strv_length(shortcuts); i++) {
                brisk_menu_window_add_shortcut(self, shortcuts[i]);
        }
        brisk_menu_window_set_filters_enabled(self, TRUE);
}

/**
 * Allow us to load the menu on the idle loop
 */
static inline gboolean inline_reload_menu(BriskMenuWindow *self)
{
        source_id = 0;
        brisk_menu_window_build(self);
        return FALSE;
}

/**
 * Handle rebuilding of tree in response to a change.
 * We automatically throttle incoming requests, so if we're sent lots of signals
 * from the underlying library consecutively (it tries to prevent most of that)
 * we restart the timeout.
 */
static inline void brisk_menu_window_reloaded(__brisk_unused__ MateMenuTree *tree, gpointer v)
{
        if (source_id > 0) {
                g_source_remove(source_id);
                source_id = 0;
        }

        source_id = g_timeout_add_full(G_PRIORITY_LOW,
                                       BRISK_RELOAD_TIME,
                                       (GSourceFunc)inline_reload_menu,
                                       v,
                                       NULL);
}

/**
 * Load the menus and place them into the window regions
 */
gboolean brisk_menu_window_load_menus(BriskMenuWindow *self)
{
        MateMenuTree *tree = NULL;

        /* Load menu */
        tree = matemenu_tree_lookup("mate-applications.menu", MATEMENU_TREE_FLAGS_NONE);
        matemenu_tree_add_monitor(tree, brisk_menu_window_reloaded, self);
        self->root = tree;

        /* Load menus on idle */
        g_idle_add((GSourceFunc)inline_reload_menu, self);
        return FALSE;
}

/**
 * brisk_menu_window_add_shortcut
 *
 * If we can create a .desktop launcher for the given name, add a new button to
 * the sidebar as a quick launch facility.
 */
static void brisk_menu_window_add_shortcut(BriskMenuWindow *self, const gchar *id)
{
        GDesktopAppInfo *info = NULL;
        GtkWidget *button = NULL;

        info = g_desktop_app_info_new(id);
        if (!info) {
                g_message("Not adding missing %s to BriskMenu", id);
                return;
        }

        button = brisk_menu_desktop_button_new(self->launcher, G_APP_INFO(info));
        gtk_widget_show_all(button);
        gtk_box_pack_start(GTK_BOX(self->sidebar), button, FALSE, FALSE, 1);
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
