/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016-2017 Ikey Doherty <ikey@solus-project.com>
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

/**
 * Begin a build of the menu structure
 */
static void brisk_menu_window_build(BriskMenuWindow *self)
{
        GtkWidget *sep = NULL;
        autofree(gstrv) *shortcuts = NULL;

        g_message("debug: menu reloaded");

        /* Nuke the current mapping */
        g_hash_table_remove_all(self->item_store);

        brisk_menu_window_set_filters_enabled(self, FALSE);

        /* Clear existing */
        brisk_menu_kill_children(GTK_CONTAINER(self->sidebar));
        brisk_menu_kill_children(GTK_CONTAINER(self->apps));

        /* Special meaning for NULL group */
        self->all_button = brisk_menu_category_button_new(NULL);
        gtk_box_pack_start(GTK_BOX(self->sidebar), self->all_button, FALSE, FALSE, 0);
        gtk_widget_show_all(self->all_button);
        brisk_menu_window_associate_category(self, self->all_button);

        /* TODO: Restore some kind of initial loading ..  */

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
 * Load the menus and place them into the window regions
 */
gboolean brisk_menu_window_load_menus(BriskMenuWindow *self)
{
        g_message("Menu loading not yet implemented!");
        brisk_menu_window_build(self);
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
