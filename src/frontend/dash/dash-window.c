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

#include "styles.h"
#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "category-button.h"
#include "dash-entry-button.h"
#include "dash-window.h"
#include <glib/gi18n.h>
BRISK_END_PEDANTIC

G_DEFINE_TYPE(BriskDashWindow, brisk_dash_window, BRISK_TYPE_MENU_WINDOW)

static void brisk_dash_window_associate_category(BriskMenuWindow *self, GtkWidget *button);
static void brisk_dash_window_on_toggled(BriskMenuWindow *self, GtkWidget *button);
static gboolean brisk_dash_window_on_enter(BriskMenuWindow *self, GdkEventCrossing *event,
                                           GtkWidget *button);
static void brisk_dash_window_load_css(GtkSettings *settings, const gchar *key, BriskDashWindow *self);
static void brisk_dash_window_key_activate(BriskDashWindow *self, gpointer v);
static void brisk_dash_window_activated(BriskMenuWindow *self, GtkFlowBoxChild *row, gpointer v);
static void brisk_dash_window_set_filters_enabled(BriskDashWindow *self, gboolean enabled);
static gboolean brisk_dash_window_filter_apps(GtkFlowBoxChild *row, gpointer v);
static gint brisk_dash_window_sort(GtkFlowBoxChild *row1, GtkFlowBoxChild *row2, gpointer v);

/**
 * brisk_dash_window_dispose:
 *
 * Clean up a BriskDashWindow instance
 */
static void brisk_dash_window_dispose(GObject *obj)
{
        BriskDashWindow *self = BRISK_DASH_WINDOW(obj);
        GdkScreen *screen = NULL;

        if (self->css) {
                screen = gtk_widget_get_screen(GTK_WIDGET(self));
                gtk_style_context_remove_provider_for_screen(screen, GTK_STYLE_PROVIDER(self->css));
                g_clear_object(&self->css);
        }

        G_OBJECT_CLASS(brisk_dash_window_parent_class)->dispose(obj);
}

static const gchar *brisk_dash_window_get_id(__brisk_unused__ BriskMenuWindow *window)
{
        return "dash";
}

static const gchar *brisk_dash_window_get_display_name(__brisk_unused__ BriskMenuWindow *window)
{
        return _("Dash");
}

static void brisk_dash_window_update_screen_position(BriskMenuWindow *self)
{
        GdkScreen *screen = NULL;
        GtkAllocation relative_alloc = { 0 };
        GdkWindow *window = NULL;
        GdkRectangle geom = { 0 };
        gint relative_x, relative_y = 0;      /* Real X, Y of the applet, on screen */
        gint window_width, window_height = 0; /* Window width & height */
        gint mon = 0;                         /* Monitor to display on */
        gint window_x, window_y = 0;          /* Target X, Y */

        if (!self->relative_to) {
                g_warning("Cannot set relative location without relative widget!");
                return;
        }

        /* Forcibly realize the applet window */
        if (!gtk_widget_get_realized(self->relative_to)) {
                gtk_widget_realize(self->relative_to);
        }

        /* Forcibly realize ourselves */
        if (!gtk_widget_get_realized(GTK_WIDGET(self))) {
                gtk_widget_realize(GTK_WIDGET(self));
        }

        gtk_widget_get_allocation(self->relative_to, &relative_alloc);

        /* Find out where we are on screen */
        window = gtk_widget_get_window(self->relative_to);
        gdk_window_get_origin(window, &relative_x, &relative_y);

        /* Grab the geometry for the monitor we're currently on */
        screen = gtk_widget_get_screen(self->relative_to);
        mon = gdk_screen_get_monitor_at_point(screen, relative_x, relative_y);
        gdk_screen_get_monitor_geometry(screen, mon, &geom);

        switch (self->position) {
        case GTK_POS_LEFT:
                window_width = geom.width - relative_alloc.width;
                window_height = geom.height;
                window_x = geom.x + relative_alloc.width;
                window_y = geom.y;
                break;
        case GTK_POS_RIGHT:
                window_width = geom.width - relative_alloc.width;
                window_height = geom.height;
                window_x = geom.x;
                window_y = geom.y;
                break;
        case GTK_POS_TOP:
                /* Top panel, appear below it */
                window_width = geom.width;
                window_height = geom.height - relative_alloc.height;
                window_x = geom.x;
                window_y = relative_y + relative_alloc.height;
                break;
        case GTK_POS_BOTTOM:
        default:
                /* Bottom panel, appear above it */
                window_width = geom.width;
                window_height = geom.height - relative_alloc.height;
                window_x = geom.x;
                window_y = geom.y;
                break;
        }

        gtk_window_set_default_size(GTK_WINDOW(self), window_width, window_height);
        gtk_window_move(GTK_WINDOW(self), window_x, window_y);
}

/**
 * Backend has new items for us, add to the global store
 */
static void brisk_dash_window_add_item(BriskMenuWindow *self, BriskItem *item,
                                       __brisk_unused__ BriskBackend *backend)
{
        BriskMenuEntryButton *button = NULL;
        const gchar *item_id = brisk_item_get_id(item);

        button = brisk_dash_entry_button_new(self->launcher, item);
        g_signal_connect_swapped(button,
                                 "show-context-menu",
                                 G_CALLBACK(brisk_menu_window_show_context),
                                 self);
        gtk_container_add(GTK_CONTAINER(BRISK_DASH_WINDOW(self)->apps), GTK_WIDGET(button));
        gtk_widget_show_all(GTK_WIDGET(button));

        g_hash_table_insert(self->item_store, g_strdup(item_id), GTK_WIDGET(button));
}

/**
 * Backend has a new sidebar section for us
 */
static void brisk_dash_window_add_section(BriskMenuWindow *self, BriskSection *section,
                                          __brisk_unused__ BriskBackend *backend)
{
        GtkWidget *button = NULL;
        const gchar *section_id = brisk_section_get_id(section);
        GtkWidget *box_target = NULL;

        /* Skip dupes. Sections are uniquely namespaced */
        if (g_hash_table_lookup(self->item_store, section_id) != NULL) {
                return;
        }

        box_target = brisk_menu_window_get_section_box(self, backend);
        gtk_orientable_set_orientation(GTK_ORIENTABLE(box_target), GTK_ORIENTATION_HORIZONTAL);

        button = brisk_dash_category_button_new(section);
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(button),
                                    GTK_RADIO_BUTTON(self->section_box_leader));
        gtk_box_pack_start(GTK_BOX(box_target), button, FALSE, FALSE, 0);
        brisk_dash_window_associate_category(self, button);
        gtk_widget_show_all(button);

        /* Avoid new dupes */
        g_hash_table_insert(self->item_store, g_strdup(section_id), button);

        brisk_menu_window_select_sections(self);
}

/**
 * A backend needs us to invalidate the filters
 */
static void brisk_dash_window_invalidate_filter(BriskMenuWindow *self,
                                                __brisk_unused__ BriskBackend *backend)
{
        gtk_flow_box_invalidate_filter(GTK_FLOW_BOX(BRISK_DASH_WINDOW(self)->apps));
        gtk_flow_box_invalidate_sort(GTK_FLOW_BOX(BRISK_DASH_WINDOW(self)->apps));
}

/**
 * A backend needs us to purge any data we have for it
 */
static void brisk_dash_window_reset(BriskMenuWindow *self, BriskBackend *backend)
{
        GtkWidget *box_target = NULL;
        GList *kids = NULL, *elem = NULL;
        const gchar *backend_id = NULL;

        backend_id = brisk_backend_get_id(backend);

        box_target = brisk_menu_window_get_section_box(self, backend);
        gtk_container_foreach(GTK_CONTAINER(box_target),
                              (GtkCallback)brisk_menu_window_remove_category,
                              self);

        /* Manual work for the items */
        kids = gtk_container_get_children(GTK_CONTAINER(BRISK_DASH_WINDOW(self)->apps));
        for (elem = kids; elem; elem = elem->next) {
                GtkWidget *row = elem->data;
                GtkWidget *child = NULL;
                BriskItem *item = NULL;
                const gchar *local_backend_id = NULL;
                const gchar *local_id = NULL;

                if (!GTK_IS_BIN(GTK_BIN(row))) {
                        continue;
                }

                child = gtk_bin_get_child(GTK_BIN(row));
                if (!BRISK_IS_MENU_ENTRY_BUTTON(child)) {
                        continue;
                }

                g_object_get(child, "item", &item, NULL);
                if (!item) {
                        g_warning("missing item for entry in backend '%s'", backend_id);
                        continue;
                }

                local_backend_id = brisk_item_get_backend_id(item);
                if (!g_str_equal(backend_id, local_backend_id)) {
                        continue;
                }
                local_id = brisk_item_get_id(item);
                g_hash_table_remove(self->item_store, local_id);
                gtk_widget_destroy(row);
        }
        g_list_free(kids);
}

/**
 * Override hiding so that we can invalidate all filters
 */
static void brisk_dash_window_hide(GtkWidget *widget)
{
        BriskDashWindow *self = NULL;
        GtkAdjustment *adjustment = NULL;
        autofree(GList) *selected_children = NULL;
        GtkWidget *selected = NULL;

        /* Have parent deal with it first */
        GTK_WIDGET_CLASS(brisk_dash_window_parent_class)->hide(widget);

        self = BRISK_DASH_WINDOW(widget);

        /* Remove search filter */
        gtk_entry_set_text(GTK_ENTRY(BRISK_MENU_WINDOW(self)->search), "");

        brisk_menu_window_select_sections(BRISK_MENU_WINDOW(self));

        /* Reset scrollbars */
        adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(self->apps_scroll));
        gtk_adjustment_set_value(adjustment, 0);
        adjustment =
            gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(self->categories_scroll));
        gtk_adjustment_set_value(adjustment, 0);

        /* Unselect any current "apps" */
        selected_children = gtk_flow_box_get_selected_children(GTK_FLOW_BOX(self->apps));
        selected = g_list_nth_data(selected_children, 0);
        if (selected) {
                gtk_flow_box_unselect_child(GTK_FLOW_BOX(self->apps), GTK_FLOW_BOX_CHILD(selected));
        }
}

/**
 * brisk_dash_window_class_init:
 *
 * Handle class initialisation
 */
static void brisk_dash_window_class_init(BriskDashWindowClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        BriskMenuWindowClass *b_class = BRISK_MENU_WINDOW_CLASS(klazz);
        GtkWidgetClass *wid_class = GTK_WIDGET_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_dash_window_dispose;

        /* Backend vtable hookup */
        b_class->get_id = brisk_dash_window_get_id;
        b_class->get_display_name = brisk_dash_window_get_display_name;
        b_class->update_screen_position = brisk_dash_window_update_screen_position;
        b_class->add_item = brisk_dash_window_add_item;
        b_class->add_section = brisk_dash_window_add_section;
        b_class->invalidate_filter = brisk_dash_window_invalidate_filter;
        b_class->reset = brisk_dash_window_reset;

        wid_class->hide = brisk_dash_window_hide;
}

/**
 * brisk_dash_window_init:
 *
 * Handle construction of the BriskDashWindow
 */
static void brisk_dash_window_init(BriskDashWindow *self)
{
        GtkWidget *layout = NULL;
        GtkWidget *widget = NULL;
        GtkWidget *scroll = NULL;
        GtkWidget *header = NULL;
        GtkWidget *content = NULL;
        GdkScreen *screen = NULL;
        GtkStyleContext *style = NULL;
        BriskMenuWindow *base = NULL;
        GtkSettings *gtk_settings = NULL;
        autofree(gchar) *txt_holder = NULL;

        base = BRISK_MENU_WINDOW(self);

        brisk_dash_window_load_css(gtk_settings, "gtk-theme-name", self);
        gtk_settings = gtk_settings_get_default();
        g_signal_connect(gtk_settings,
                         "notify::gtk-theme-name",
                         G_CALLBACK(brisk_dash_window_load_css),
                         self);

        style = gtk_widget_get_style_context(GTK_WIDGET(self));
        gtk_style_context_add_class(style, BRISK_STYLE_MAIN);
        gtk_style_context_add_class(style, BRISK_STYLE_DASH);

        brisk_menu_window_configure_grabs(base);

        layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_container_add(GTK_CONTAINER(self), layout);

        header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        style = gtk_widget_get_style_context(GTK_WIDGET(header));
        gtk_style_context_add_class(style, "header");
        gtk_box_pack_start(GTK_BOX(layout), header, FALSE, FALSE, 0);

        /* Create search entry */
        widget = gtk_entry_new();
        gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget),
                                          GTK_ENTRY_ICON_PRIMARY,
                                          "edit-find-symbolic");
        gtk_entry_set_icon_from_icon_name(GTK_ENTRY(widget),
                                          GTK_ENTRY_ICON_SECONDARY,
                                          "edit-clear-symbolic");
        gtk_box_pack_start(GTK_BOX(header), widget, FALSE, FALSE, 20);
        gtk_entry_set_width_chars(GTK_ENTRY(widget), 55);
        gtk_widget_set_valign(widget, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(widget, GTK_ALIGN_CENTER);
        /* Translators: This is the text shown in the "search box" with no content */
        txt_holder = g_strdup_printf("%s\u2026", _("Type to search"));
        gtk_entry_set_placeholder_text(GTK_ENTRY(widget), txt_holder);
        base->search = widget;
        g_signal_connect_swapped(widget, "changed", G_CALLBACK(brisk_menu_window_search), self);
        g_signal_connect_swapped(widget,
                                 "activate",
                                 G_CALLBACK(brisk_dash_window_key_activate),
                                 self);
        g_signal_connect(widget, "icon-press", G_CALLBACK(brisk_menu_window_clear_search), self);

        /* Scrollbar for categories */
        scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_NONE);
        gtk_box_pack_start(GTK_BOX(header), scroll, FALSE, FALSE, 0);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_NEVER);
        gtk_widget_set_can_focus(scroll, FALSE);
        gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), TRUE);
        self->categories_scroll = scroll;

        /* Container for categories */
        widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_container_add(GTK_CONTAINER(scroll), widget);
        base->section_box_holder = widget;

        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, "section-box-holder");

        /* Content layout */
        content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_box_pack_start(GTK_BOX(layout), content, TRUE, TRUE, 0);

        /* Scrollbar for apps */
        scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll), GTK_SHADOW_NONE);
        gtk_box_pack_start(GTK_BOX(content), scroll, TRUE, TRUE, 0);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                       GTK_POLICY_NEVER,
                                       GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(scroll), FALSE);
        self->apps_scroll = scroll;

        /* Application launcher display */
        widget = gtk_flow_box_new();
        gtk_widget_set_margin_top(widget, 50);
        gtk_widget_set_margin_bottom(widget, 50);
        gtk_widget_set_margin_start(widget, 150);
        gtk_widget_set_margin_end(widget, 150);
        gtk_container_add(GTK_CONTAINER(scroll), widget);
        self->apps = widget;
        g_object_set(self->apps, "halign", GTK_ALIGN_FILL, "valign", GTK_ALIGN_START, NULL);
        gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(widget), 80);
        gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(widget), 60);
        gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(widget), TRUE);
        gtk_flow_box_set_activate_on_single_click(GTK_FLOW_BOX(self->apps), TRUE);
        gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(self->apps), GTK_SELECTION_SINGLE);
        g_signal_connect_swapped(self->apps,
                                 "child-activated",
                                 G_CALLBACK(brisk_dash_window_activated),
                                 self);

        screen = gtk_widget_get_screen(widget);
        GdkVisual *vis = gdk_screen_get_rgba_visual(screen);
        if (vis) {
                gtk_widget_set_visual(GTK_WIDGET(self), vis);
        }

        brisk_dash_window_set_filters_enabled(self, FALSE);

        /* Special leader to control group association, hidden from view */
        base->section_box_leader = gtk_radio_button_new(NULL);
        gtk_box_pack_start(GTK_BOX(base->section_box_holder),
                           base->section_box_leader,
                           FALSE,
                           FALSE,
                           0);
        gtk_widget_set_no_show_all(base->section_box_leader, TRUE);
        gtk_widget_hide(base->section_box_leader);

        brisk_dash_window_set_filters_enabled(self, TRUE);

        /* Hook up keyboard events */
        g_signal_connect(self,
                         "key-release-event",
                         G_CALLBACK(brisk_menu_window_key_release),
                         NULL);
        g_signal_connect(self, "key-press-event", G_CALLBACK(brisk_menu_window_key_press), NULL);

        brisk_menu_window_init_backends(base);

        gtk_widget_show_all(layout);
}

/**
 * brisk_dash_window_new:
 *
 * Return a newly created BriskDashWindow
 */
BriskMenuWindow *brisk_dash_window_new(GtkWidget *relative_to)
{
        return g_object_new(BRISK_TYPE_DASH_WINDOW,
                            "type",
                            GTK_WINDOW_POPUP,
                            "relative-to",
                            relative_to,
                            NULL);
}

/**
 * brisk_dash_window_associate_category:
 *
 * This will hook up the category button for events to enable us to filter the
 * list based on the active category.
 */
static void brisk_dash_window_associate_category(BriskMenuWindow *self, GtkWidget *button)
{
        g_signal_connect_swapped(button, "toggled", G_CALLBACK(brisk_dash_window_on_toggled), self);
        g_signal_connect_swapped(button,
                                 "enter-notify-event",
                                 G_CALLBACK(brisk_dash_window_on_enter),
                                 self);
}

/**
 * Fired by clicking a category button
 */
static void brisk_dash_window_on_toggled(BriskMenuWindow *self, GtkWidget *button)
{
        BriskDashCategoryButton *cat = NULL;

        /* Skip a double signal due to using a group */
        if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
                return;
        }

        cat = BRISK_DASH_CATEGORY_BUTTON(button);
        g_object_get(cat, "section", &self->active_section, NULL);

        /* Start the filter. */
        brisk_dash_window_invalidate_filter(self, NULL);
}

/**
 * Fired by entering into the category button with a roll over
 */
static gboolean brisk_dash_window_on_enter(BriskMenuWindow *self,
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
 * Load up the CSS assets
 */
static void brisk_dash_window_load_css(GtkSettings *settings, const gchar *key, BriskDashWindow *self)
{
        GtkCssProvider *css = NULL;
        GtkStyleContext *context = NULL;
        autofree(GFile) *file = NULL;
        autofree(GError) *err = NULL;
        GdkScreen *screen = NULL;
        GdkRGBA color;

        file = g_file_new_for_uri("resource://com/solus-project/brisk/menu/dash/styling.css");

        context = gtk_widget_get_style_context(GTK_WIDGET(self));
        if (gtk_style_context_lookup_color(context, "dark_bg_color", &color)) {
                file = g_file_new_for_uri("resource://com/solus-project/brisk/menu/dash/styling-light.css");
        }
        
        if (!file) {
                return;
        }

        screen = gtk_widget_get_screen(GTK_WIDGET(self));

        if (self->css) {
                gtk_style_context_remove_provider_for_screen(screen, GTK_STYLE_PROVIDER(self->css));
                g_clear_object(&self->css);
        }

        css = gtk_css_provider_new();
        self->css = css;

        gtk_style_context_add_provider_for_screen(screen,
                                                  GTK_STYLE_PROVIDER(css),
                                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

        if (!gtk_css_provider_load_from_file(css, file, &err)) {
                g_warning("Failed to load CSS: %s\n", err->message);
                return;
        }
}

static void brisk_dash_window_key_activate(BriskDashWindow *self, __brisk_unused__ gpointer v)
{
        autofree(GList) *kids = NULL;
        GList *elem = NULL;
        BriskMenuEntryButton *button = NULL;

        kids = gtk_container_get_children(GTK_CONTAINER(self->apps));

        for (elem = kids; elem; elem = elem->next) {
                GtkWidget *widget = elem->data;

                if (!gtk_widget_get_visible(widget) || !gtk_widget_get_child_visible(widget)) {
                        continue;
                }

                button = BRISK_MENU_ENTRY_BUTTON(gtk_bin_get_child(GTK_BIN(widget)));
                break;
        }
        if (!button) {
                return;
        }
        brisk_menu_entry_button_launch(button);
}

static void brisk_dash_window_activated(__brisk_unused__ BriskMenuWindow *self,
                                        GtkFlowBoxChild *row, __brisk_unused__ gpointer v)
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

/**
 * Enable or disable the filters between building of the menus
 */
static void brisk_dash_window_set_filters_enabled(BriskDashWindow *self, gboolean enabled)
{
        BRISK_MENU_WINDOW(self)->filtering = enabled;
        if (enabled) {
                gtk_flow_box_set_filter_func(GTK_FLOW_BOX(self->apps),
                                             brisk_dash_window_filter_apps,
                                             self,
                                             NULL);
                gtk_flow_box_set_sort_func(GTK_FLOW_BOX(self->apps),
                                           brisk_dash_window_sort,
                                           self,
                                           NULL);
                return;
        }
        gtk_flow_box_set_filter_func(GTK_FLOW_BOX(self->apps), NULL, NULL, NULL);
        gtk_flow_box_set_sort_func(GTK_FLOW_BOX(self->apps), NULL, NULL, NULL);
}

/**
 * brisk_dash_window_filter_apps:
 *
 * Responsible for filtering the selection based on active group or search
 * term.
 */
__brisk_pure__ static gboolean brisk_dash_window_filter_apps(GtkFlowBoxChild *row, gpointer v)
{
        BriskMenuWindow *self = NULL;
        GtkWidget *child = NULL;

        self = BRISK_MENU_WINDOW(v);

        if (!self->filtering) {
                return FALSE;
        }

        /* Grab our Entry widget */
        child = gtk_bin_get_child(GTK_BIN(row));

        return brisk_menu_window_filter_apps(self, child);
}

static gint brisk_dash_window_sort(GtkFlowBoxChild *row1, GtkFlowBoxChild *row2, gpointer v)
{
        GtkWidget *child1, *child2 = NULL;
        BriskItem *itemA, *itemB = NULL;
        autofree(gchar) *nameA = NULL;
        autofree(gchar) *nameB = NULL;
        BriskMenuWindow *self = NULL;

        self = BRISK_MENU_WINDOW(v);

        child1 = gtk_bin_get_child(GTK_BIN(row1));
        child2 = gtk_bin_get_child(GTK_BIN(row2));

        g_object_get(child1, "item", &itemA, NULL);
        g_object_get(child2, "item", &itemB, NULL);

        return brisk_menu_window_sort(self, itemA, itemB);
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
