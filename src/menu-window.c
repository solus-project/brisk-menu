/*
 * This file is part of mate-solmenu.
 * 
 * Copyright © 2016 Ikey Doherty <ikey@solus-project.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

SOLUS_BEGIN_PEDANTIC
#include "category-button.h"
#include "menu-private.h"
#include "menu-window.h"
#include <gtk/gtk.h>
SOLUS_END_PEDANTIC

G_DEFINE_TYPE(SolMenuWindow, sol_menu_window, GTK_TYPE_WINDOW)

/**
 * sol_menu_window_new:
 *
 * Construct a new SolMenuWindow object
 */
GtkWidget *sol_menu_window_new()
{
        return g_object_new(SOL_TYPE_MENU_WINDOW, NULL);
}

/**
 * sol_menu_window_dispose:
 *
 * Clean up a SolMenuWindow instance
 */
static void sol_menu_window_dispose(GObject *obj)
{
        SolMenuWindow *self = SOL_MENU_WINDOW(obj);

        g_message("debug: cleaning up");

        g_clear_pointer(&self->root, matemenu_tree_unref);
        g_clear_pointer(&self->search_term, g_free);

        G_OBJECT_CLASS(sol_menu_window_parent_class)->dispose(obj);
}

/**
 * sol_menu_window_class_init:
 *
 * Handle class initialisation
 */
static void sol_menu_window_class_init(SolMenuWindowClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = sol_menu_window_dispose;
}

/**
 * sol_menu_window_init:
 *
 * Handle construction of the SolMenuWindow
 */
static void sol_menu_window_init(SolMenuWindow *self)
{
        GtkWidget *layout = NULL;
        GtkWidget *widget = NULL;
        GtkWidget *content = NULL;
        GtkWidget *scroll = NULL;
        GtkStyleContext *style = NULL;

        gtk_window_set_decorated(GTK_WINDOW(self), FALSE);

        /* Create the main layout (Vertical search/content */
        layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_add(GTK_CONTAINER(self), layout);

        /* Create search entry - but not GtkSearchEntry to avoid rounding in themes */
        widget = gtk_entry_new();
        gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget),
                                          GTK_ENTRY_ICON_PRIMARY,
                                          "edit-find-symbolic");
        g_object_set(widget, "margin", 2, NULL);

        gtk_box_pack_start(GTK_BOX(layout), widget, FALSE, FALSE, 0);
        gtk_entry_set_placeholder_text(GTK_ENTRY(widget), "Type to search\u2026");
        self->search = widget;
        g_signal_connect_swapped(widget, "changed", G_CALLBACK(sol_menu_window_search), self);

        /* Content layout */
        content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(layout), content, TRUE, TRUE, 0);

        /* Sidebar for categories */
        widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                       GTK_POLICY_NEVER,
                                       GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), FALSE);
        gtk_box_pack_start(GTK_BOX(content), scroll, TRUE, TRUE, 0);
        self->sidebar = widget;
        gtk_container_add(GTK_CONTAINER(scroll), widget);

        /* Stick a vsep in for visual separation */
        widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_box_pack_start(GTK_BOX(content), widget, FALSE, FALSE, 0);

        /* Scrollbar for apps */
        scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_box_pack_start(GTK_BOX(content), scroll, TRUE, TRUE, 0);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                       GTK_POLICY_NEVER,
                                       GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), FALSE);

        /* Application launcher display */
        widget = gtk_list_box_new();
        gtk_list_box_set_filter_func(GTK_LIST_BOX(widget), sol_menu_window_filter_apps, self, NULL);
        gtk_container_add(GTK_CONTAINER(scroll), widget);
        self->apps = widget;

        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, "view");
        gtk_style_context_add_class(style, "content-view");
        gtk_style_context_remove_class(style, "background");

        gtk_window_set_default_size(GTK_WINDOW(self), 300, 510);
        g_object_set(layout, "margin-top", 3, "margin-left", 3, "margin-right", 3, NULL);

        sol_menu_window_load_menus(self);
}

/**
 * Fired by clicking a category button
 */
static void sol_menu_window_on_toggled(SolMenuWindow *self, GtkWidget *button)
{
        SolMenuCategoryButton *cat = NULL;

        /* Skip a double signal due to using a group */
        if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
                return;
        }

        cat = SOL_MENU_CATEGORY_BUTTON(button);
        g_object_get(cat, "group", &self->active_group, NULL);

        if (!self->active_group) {
                g_message("debug: active group is: All");
        } else {
                g_message("debug: active group is: %s",
                          matemenu_tree_directory_get_name(self->active_group));
        }

        /* Start the filter. */
        gtk_list_box_invalidate_filter(GTK_LIST_BOX(self->apps));
}

/**
 * sol_menu_window_associate_category:
 *
 * This will hook up the category button for events to enable us to filter the
 * list based on the active category.
 */
void sol_menu_window_associate_category(SolMenuWindow *self, GtkWidget *button)
{
        g_signal_connect_swapped(button, "toggled", G_CALLBACK(sol_menu_window_on_toggled), self);
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
