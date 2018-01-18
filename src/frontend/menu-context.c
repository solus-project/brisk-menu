/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2017-2018 Brisk Menu Developers
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

#define BRISK_ACTION_GROUP "brisk-context-items"

DEF_AUTOFREE(GMenu, g_object_unref)

/**
 * brisk_menu_window_context_hide:
 *
 * Existing menu has gone bye-bye, so clear it up. Typically this only happens
 * when the new menu is about to be inserted, and not when the menu is actually
 * closed.
 */
static void brisk_menu_window_context_hide(GtkWidget *attached, GtkMenu *menu)
{
        BriskMenuWindow *self = BRISK_MENU_WINDOW(attached);

        /* Pop any existing action group */
        gtk_widget_insert_action_group(attached, BRISK_ACTION_GROUP, NULL);
        gtk_widget_destroy(GTK_WIDGET(menu));
        self->context_group = NULL;
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
        GSimpleActionGroup *group = NULL;
        autofree(GMenu) *simple_menu = NULL;

        g_clear_pointer(&self->context_menu, gtk_widget_destroy);

        group = g_simple_action_group_new();
        self->context_group = G_ACTION_GROUP(group);

        /* For now, iterate all the backends and stick the actions in */
        g_hash_table_iter_init(&iter, self->backends);
        while (g_hash_table_iter_next(&iter, (void **)&key, (void **)&backend)) {
                autofree(GMenu) *section = NULL;

                section = brisk_backend_get_item_actions(backend, item, G_ACTION_GROUP(group));
                if (!section) {
                        continue;
                }

                if (!simple_menu) {
                        simple_menu = g_menu_new();
                }

                /* Automatically separated, no titles */
                g_menu_append_section(simple_menu, NULL, G_MENU_MODEL(section));
        }

        /* No sense displaying an empty menu */
        if (!simple_menu) {
                return;
        }

        /* Push in the action group for invokables */
        gtk_widget_insert_action_group(GTK_WIDGET(self), BRISK_ACTION_GROUP, G_ACTION_GROUP(group));

        self->context_menu = gtk_menu_new_from_model(G_MENU_MODEL(simple_menu));
        gtk_menu_attach_to_widget(GTK_MENU(self->context_menu),
                                  GTK_WIDGET(self),
                                  brisk_menu_window_context_hide);

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
