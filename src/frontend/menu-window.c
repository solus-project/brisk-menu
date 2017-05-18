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

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "category-button.h"
#include "entry-button.h"
#include "launcher.h"
#include "menu-private.h"
#include "menu-window.h"
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

G_DEFINE_TYPE(BriskMenuWindow, brisk_menu_window, GTK_TYPE_WINDOW)

static void brisk_menu_window_load_css(BriskMenuWindow *self);
static void brisk_menu_window_hide(GtkWidget *widget);

/**
 * brisk_menu_window_new:
 *
 * Construct a new BriskMenuWindow object
 */
GtkWidget *brisk_menu_window_new()
{
        return g_object_new(BRISK_TYPE_MENU_WINDOW, "type", GTK_WINDOW_POPUP, NULL);
}

/**
 * brisk_menu_window_dispose:
 *
 * Clean up a BriskMenuWindow instance
 */
static void brisk_menu_window_dispose(GObject *obj)
{
        BriskMenuWindow *self = BRISK_MENU_WINDOW(obj);
        GdkScreen *screen = NULL;

        g_message("debug: cleaning up");

        if (self->css) {
                screen = gtk_widget_get_screen(GTK_WIDGET(self));
                gtk_style_context_remove_provider_for_screen(screen, GTK_STYLE_PROVIDER(self->css));
                g_clear_object(&self->css);
        }

        g_clear_pointer(&self->search_term, g_free);
        g_clear_object(&self->launcher);
        g_clear_object(&self->session);
        g_clear_object(&self->saver);
        g_clear_object(&self->settings);
        g_clear_object(&self->apps_backend);
        g_clear_pointer(&self->item_store, g_hash_table_unref);

        G_OBJECT_CLASS(brisk_menu_window_parent_class)->dispose(obj);
}

/**
 * brisk_menu_window_class_init:
 *
 * Handle class initialisation
 */
static void brisk_menu_window_class_init(BriskMenuWindowClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        GtkWidgetClass *wid_class = GTK_WIDGET_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_menu_window_dispose;

        /* widget vtable */
        wid_class->hide = brisk_menu_window_hide;
}

/**
 * brisk_menu_window_init:
 *
 * Handle construction of the BriskMenuWindow
 */
static void brisk_menu_window_init(BriskMenuWindow *self)
{
        GtkWidget *layout = NULL;
        GtkWidget *widget = NULL;
        GtkWidget *content = NULL;
        GtkWidget *scroll = NULL;
        GtkStyleContext *style = NULL;

        self->launcher = brisk_menu_launcher_new();
        brisk_menu_window_load_css(self);
        brisk_menu_window_init_settings(self);
        self->item_store = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

        gtk_window_set_decorated(GTK_WINDOW(self), FALSE);
        gtk_window_set_type_hint(GTK_WINDOW(self), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
        gtk_window_set_skip_pager_hint(GTK_WINDOW(self), TRUE);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(self), TRUE);
        style = gtk_widget_get_style_context(GTK_WIDGET(self));
        gtk_style_context_add_class(style, BRISK_STYLE_MAIN);

        /* Hook up grabs */
        brisk_menu_window_configure_grabs(self);

        /* Create the main layout (Vertical search/content */
        layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_container_add(GTK_CONTAINER(self), layout);

        /* Create search entry - but not GtkSearchEntry to avoid rounding in themes */
        widget = gtk_entry_new();
        gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget),
                                          GTK_ENTRY_ICON_PRIMARY,
                                          "edit-find-symbolic");
        gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget),
                                          GTK_ENTRY_ICON_SECONDARY,
                                          "edit-clear-symbolic");

        gtk_box_pack_start(GTK_BOX(layout), widget, FALSE, FALSE, 0);
        gtk_entry_set_placeholder_text(GTK_ENTRY(widget), "Type to search\u2026");
        self->search = widget;
        g_signal_connect_swapped(widget, "changed", G_CALLBACK(brisk_menu_window_search), self);
        g_signal_connect_swapped(widget,
                                 "activate",
                                 G_CALLBACK(brisk_menu_window_key_activate),
                                 self);
        g_signal_connect(widget, "icon-press", G_CALLBACK(brisk_menu_window_clear_search), self);

        /* Content layout */
        content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
        gtk_box_pack_start(GTK_BOX(layout), content, TRUE, TRUE, 0);

        /* Sidebar for categories */
        widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                       GTK_POLICY_NEVER,
                                       GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), FALSE);
        self->sidebar = widget;
        style = gtk_widget_get_style_context(self->sidebar);
        gtk_style_context_add_class(style, BRISK_STYLE_SIDEBAR);
        self->sidebar_scroll = scroll;
        gtk_container_add(GTK_CONTAINER(scroll), widget);

        /* Create a wrapper for the categories */
        widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_box_pack_start(GTK_BOX(widget), scroll, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(content), widget, TRUE, TRUE, 0);
        self->sidebar_wrap = widget;

        /* Scrollbar for apps */
        scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_IN);
        gtk_box_pack_start(GTK_BOX(content), scroll, TRUE, TRUE, 0);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                       GTK_POLICY_NEVER,
                                       GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), FALSE);
        self->apps_scroll = scroll;

        /* Application launcher display */
        widget = gtk_list_box_new();
        gtk_container_add(GTK_CONTAINER(scroll), widget);
        self->apps = widget;
        gtk_list_box_set_activate_on_single_click(GTK_LIST_BOX(self->apps), TRUE);
        gtk_list_box_set_selection_mode(GTK_LIST_BOX(self->apps), GTK_SELECTION_SINGLE);
        g_signal_connect_swapped(self->apps,
                                 "row-activated",
                                 G_CALLBACK(brisk_menu_window_activated),
                                 self);

        /* Style up the app box */
        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, BRISK_STYLE_APPS_LIST);
        gtk_style_context_add_class(style, "view");
        gtk_style_context_add_class(style, "content-view");
        gtk_style_context_remove_class(style, "background");

        /* Add a placeholder when there are no apps for current search term */
        widget = gtk_label_new("<big>Sorry, no items found</big>");
        gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
        g_object_set(widget,
                     "halign",
                     GTK_ALIGN_CENTER,
                     "valign",
                     GTK_ALIGN_START,
                     "margin",
                     6,
                     NULL);
        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, "dim-label");
        gtk_list_box_set_placeholder(GTK_LIST_BOX(self->apps), widget);
        gtk_widget_show_all(widget);

        /* Add session controls */
        brisk_menu_window_setup_session_controls(self);

        gtk_window_set_default_size(GTK_WINDOW(self), 300, 510);
        g_object_set(layout, "margin", 3, NULL);

        /* Hook up keyboard events */
        g_signal_connect(self,
                         "key-release-event",
                         G_CALLBACK(brisk_menu_window_key_release),
                         NULL);

        brisk_menu_window_init_backends(self);

        /* Hook up dbus later on */
        g_idle_add((GSourceFunc)brisk_menu_window_setup_session, self);
        /* Start with all content parts "shown" */
        gtk_widget_show_all(layout);
}

/**
 * Fired by clicking a category button
 */
static void brisk_menu_window_on_toggled(BriskMenuWindow *self, GtkWidget *button)
{
        BriskMenuCategoryButton *cat = NULL;

        /* Skip a double signal due to using a group */
        if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
                return;
        }

        cat = BRISK_MENU_CATEGORY_BUTTON(button);
        g_object_get(cat, "section", &self->active_section, NULL);

        /* Start the filter. */
        gtk_list_box_invalidate_filter(GTK_LIST_BOX(self->apps));
}

/**
 * Fired by entering into the category button with a roll over
 */
static gboolean brisk_menu_window_on_enter(BriskMenuWindow *self,
                                           __brisk_unused__ GdkEventCrossing *event,
                                           GtkWidget *button)
{
        GtkToggleButton *but = GTK_TOGGLE_BUTTON(button);

        /* Whether we're in rollover mode */
        if (!self->rollover) {
                return GDK_EVENT_PROPAGATE;
        }

        if (gtk_toggle_button_get_active(but) || !gtk_widget_get_visible(button)) {
                return GDK_EVENT_PROPAGATE;
        }

        /* Force activation through rollover */
        gtk_toggle_button_set_active(but, TRUE);

        return GDK_EVENT_PROPAGATE;
}

/**
 * brisk_menu_window_associate_category:
 *
 * This will hook up the category button for events to enable us to filter the
 * list based on the active category.
 */
void brisk_menu_window_associate_category(BriskMenuWindow *self, GtkWidget *button)
{
        g_signal_connect_swapped(button, "toggled", G_CALLBACK(brisk_menu_window_on_toggled), self);
        g_signal_connect_swapped(button,
                                 "enter-notify-event",
                                 G_CALLBACK(brisk_menu_window_on_enter),
                                 self);
}

/**
 * Load up the CSS assets
 */
static void brisk_menu_window_load_css(BriskMenuWindow *self)
{
        GtkCssProvider *css = NULL;
        autofree(GFile) *file = NULL;
        autofree(GError) *err = NULL;
        GdkScreen *screen = NULL;

        file = g_file_new_for_uri("resource://com/solus-project/brisk/menu/styling.css");
        if (!file) {
                return;
        }

        css = gtk_css_provider_new();
        self->css = css;
        screen = gtk_widget_get_screen(GTK_WIDGET(self));
        gtk_style_context_add_provider_for_screen(screen,
                                                  GTK_STYLE_PROVIDER(css),
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        if (!gtk_css_provider_load_from_file(css, file, &err)) {
                g_warning("Failed to load CSS: %s\n", err->message);
                return;
        }
}

/**
 * Override hiding so that we can invalidate all filters
 */
static void brisk_menu_window_hide(GtkWidget *widget)
{
        BriskMenuWindow *self = NULL;
        GtkAdjustment *adjustment = NULL;

        /* Have parent deal with it first */
        GTK_WIDGET_CLASS(brisk_menu_window_parent_class)->hide(widget);

        self = BRISK_MENU_WINDOW(widget);

        /* Remove search filter */
        gtk_entry_set_text(GTK_ENTRY(self->search), "");

        /* Force activate the All button */
        if (self->all_button) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(self->all_button), TRUE);
        }

        /* Reset scrollbars */
        adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(self->apps_scroll));
        gtk_adjustment_set_value(adjustment, 0);
        adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(self->sidebar_scroll));
        gtk_adjustment_set_value(adjustment, 0);

        /* Unselect any current "apps" */
        gtk_list_box_select_row(GTK_LIST_BOX(self->apps), NULL);
}

/**
 * Enable or disable the filters between building of the menus
 */
void brisk_menu_window_set_filters_enabled(BriskMenuWindow *self, gboolean enabled)
{
        self->filtering = enabled;
        if (enabled) {
                gtk_list_box_set_filter_func(GTK_LIST_BOX(self->apps),
                                             brisk_menu_window_filter_apps,
                                             self,
                                             NULL);
                gtk_list_box_set_sort_func(GTK_LIST_BOX(self->apps),
                                           brisk_menu_window_sort,
                                           self,
                                           NULL);
                return;
        }
        gtk_list_box_set_filter_func(GTK_LIST_BOX(self->apps), NULL, NULL, NULL);
        gtk_list_box_set_sort_func(GTK_LIST_BOX(self->apps), NULL, NULL, NULL);
}

void brisk_menu_window_activated(__brisk_unused__ BriskMenuWindow *self, GtkListBoxRow *row,
                                 __brisk_unused__ gpointer v)
{
        BriskMenuEntryButton *button = NULL;
        GtkWidget *child = NULL;

        child = gtk_bin_get_child(GTK_BIN(row));
        if (!child) {
                return;
        }
        if (!BRISK_IS_MENU_ENTRY_BUTTON(child)) {
                return;
        }
        button = BRISK_MENU_ENTRY_BUTTON(child);
        brisk_menu_entry_button_launch(button);
}

void brisk_menu_window_set_orient(BriskMenuWindow *self, MatePanelAppletOrient orient)
{
        self->orient = orient;
        brisk_menu_window_update_search(self, self->orient);
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
