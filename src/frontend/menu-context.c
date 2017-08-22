/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2017 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "menu-private.h"
#include "menu-window.h"
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

static void brisk_menu_entry_menu_item_activated(GAction *action, __brisk_unused__ gpointer v)
{
        g_action_activate(action, NULL);
}

/**
 * brisk_menu_window_show_context:
 *
 * Menu button has requested a context menu be shown for the given item
 *
 * WARNING: this is leaky right now and is a port of the original PR behaviour
 */
void brisk_menu_window_show_context(BriskMenuWindow *self, BriskItem *item,
                                    __brisk_unused__ BriskMenuEntryButton *button)
{
        GHashTableIter iter = { 0 };
        __brisk_unused__ gpointer key;
        BriskBackend *backend = NULL;
        gboolean had_add = FALSE;

        /* TODO: consider GMenu + action groups ... */
        g_clear_pointer(&self->context_menu, gtk_widget_destroy);
        self->context_menu = gtk_menu_new();

        /* For now, iterate all the backends and stick the actions in */
        g_hash_table_iter_init(&iter, self->backends);
        while (g_hash_table_iter_next(&iter, (void **)&key, (void **)&backend)) {
                autofree(GSList) *actions = brisk_backend_get_item_actions(backend, item);

                if (!actions) {
                        continue;
                }

                /* Whack a separator in */
                if (!had_add) {
                        GtkWidget *separator = gtk_separator_menu_item_new();
                        gtk_menu_shell_append(GTK_MENU_SHELL(self->context_menu), separator);
                        gtk_widget_show(GTK_WIDGET(separator));
                }

                for (GSList *node = actions; node; node = node->next) {
                        const gchar *name = g_action_get_name(G_ACTION(node->data));
                        GtkWidget *menu_item = gtk_menu_item_new_with_label(name);
                        gtk_menu_shell_append(GTK_MENU_SHELL(self->context_menu), menu_item);
                        gtk_widget_show(GTK_WIDGET(menu_item));
                        g_signal_connect_swapped(menu_item,
                                                 "activate",
                                                 G_CALLBACK(brisk_menu_entry_menu_item_activated),
                                                 node->data);
                }

                had_add = TRUE;
        }

        /* Show it now */
        gtk_menu_popup(GTK_MENU(self->context_menu),
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       GDK_BUTTON_SECONDARY,
                       GDK_CURRENT_TIME);
}

/**
* brisk_menu_window_configure_context:
*
* Set up the basics for handling context menus
*/
void brisk_menu_window_configure_context(__brisk_unused__ BriskMenuWindow *self)
{
        /* TODO: Init bits */
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
