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

#include "styles.h"
#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "../menu-private.h"
#include "category-button.h"
#include "classic-entry-button.h"
#include "classic-window.h"
#include "desktop-button.h"
#include "sidebar-scroller.h"

#include <gio/gdesktopappinfo.h>
#include <glib/gi18n.h>
BRISK_END_PEDANTIC

G_DEFINE_TYPE(BriskClassicWindow, brisk_classic_window, BRISK_TYPE_MENU_WINDOW)

static void brisk_classic_window_associate_category(BriskMenuWindow *self, GtkWidget *button);
static void brisk_classic_window_on_toggled(BriskMenuWindow *self, GtkWidget *button);
static gboolean brisk_classic_window_on_enter(BriskMenuWindow *self, GdkEventCrossing *event,
                                              GtkWidget *button);
static void brisk_classic_window_load_css(BriskClassicWindow *self);
static void brisk_classic_window_key_activate(BriskClassicWindow *self, gpointer v);
static void brisk_classic_window_activated(BriskMenuWindow *self, GtkListBoxRow *row, gpointer v);
static void brisk_classic_window_setup_session_controls(BriskClassicWindow *self);
static void brisk_classic_window_build_sidebar(BriskMenuWindow *self);
static void brisk_classic_window_add_shortcut(BriskMenuWindow *self, const gchar *id);
static void brisk_classic_window_set_filters_enabled(BriskClassicWindow *self, gboolean enabled);
static gboolean brisk_classic_window_filter_apps(GtkListBoxRow *row, gpointer v);
static gint brisk_classic_window_sort(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer v);

/**
 * brisk_classic_window_dispose:
 *
 * Clean up a BriskClassicWindow instance
 */
static void brisk_classic_window_dispose(GObject *obj)
{
        BriskClassicWindow *self = BRISK_CLASSIC_WINDOW(obj);
        GdkScreen *screen = NULL;

        if (self->css) {
                screen = gtk_widget_get_screen(GTK_WIDGET(self));
                gtk_style_context_remove_provider_for_screen(screen, GTK_STYLE_PROVIDER(self->css));
                g_clear_object(&self->css);
        }

        G_OBJECT_CLASS(brisk_classic_window_parent_class)->dispose(obj);
}

static const gchar *brisk_classic_window_get_id(__brisk_unused__ BriskMenuWindow *window)
{
        return "classic";
}

static const gchar *brisk_classic_window_get_display_name(__brisk_unused__ BriskMenuWindow *window)
{
        return _("Classic");
}

static void brisk_classic_window_update_screen_position(BriskMenuWindow *self)
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

        /* Find out the window size */
        gtk_window_get_size(GTK_WINDOW(self), &window_width, &window_height);

        /* Grab the geometry for the monitor we're currently on */
        screen = gtk_widget_get_screen(self->relative_to);
        mon = gdk_screen_get_monitor_at_point(screen, relative_x, relative_y);
        gdk_screen_get_monitor_geometry(screen, mon, &geom);

        switch (self->position) {
        case GTK_POS_LEFT:
                /* Left vertical panel, appear to the RHS of it */
                window_x = relative_x + relative_alloc.width;
                window_y = relative_y;
                break;
        case GTK_POS_RIGHT:
                /* Right vertical panel, appear to the LHS of it */
                window_x = relative_x - window_width;
                window_y = relative_y;
                break;
        case GTK_POS_TOP:
                /* Top panel, appear below it */
                window_x = relative_x;
                window_y = relative_y + relative_alloc.height;
                break;
        case GTK_POS_BOTTOM:
        default:
                /* Bottom panel, appear above it */
                window_x = relative_x;
                window_y = relative_y - window_height;
                break;
        }

        /* Bound the right side */
        if (window_x + window_width > (geom.x + geom.width)) {
                window_x = (geom.x + geom.width) - window_width;
                if (self->position == GTK_POS_RIGHT) {
                        window_x -= relative_alloc.width;
                }
        }

        /* Bound the left side */
        if (window_x < geom.x) {
                window_x = geom.x;
                if (self->position == GTK_POS_LEFT) {
                        window_x -= relative_alloc.width;
                }
        }

        gtk_window_move(GTK_WINDOW(self), window_x, window_y);
}

/**
 * Update the position of the search bar in accordance with settings
 */
static void brisk_classic_window_update_search(BriskMenuWindow *self)
{
        SearchPosition search_position = self->search_position;
        GtkWidget *layout = NULL;
        gint n_pos = 0;

        layout = gtk_bin_get_child(GTK_BIN(self));

        if (search_position < SEARCH_POS_MIN || search_position >= SEARCH_POS_MAX) {
                search_position = SEARCH_POS_AUTOMATIC;
        }

        switch (search_position) {
        case SEARCH_POS_AUTOMATIC:
                /* Top panel, bottom search. Bottom panel, top search */
                n_pos = self->position == GTK_POS_TOP ? 1 : 0;
                break;
        case SEARCH_POS_TOP:
                n_pos = 0;
                break;
        case SEARCH_POS_BOTTOM:
        default:
                n_pos = 1;
                break;
        }

        gtk_container_child_set(GTK_CONTAINER(layout), self->search, "position", n_pos, NULL);
}

/**
 * Backend has new items for us, add to the global store
 */
static void brisk_classic_window_add_item(BriskMenuWindow *self, BriskItem *item,
                                          __brisk_unused__ BriskBackend *backend)
{
        GtkWidget *button = NULL;
        const gchar *item_id = brisk_item_get_id(item);

        button = brisk_classic_entry_button_new(self->launcher, item);
        g_signal_connect_swapped(button,
                                 "show-context-menu",
                                 G_CALLBACK(brisk_menu_window_show_context),
                                 self);
        gtk_container_add(GTK_CONTAINER(BRISK_CLASSIC_WINDOW(self)->apps), button);
        gtk_widget_show_all(button);

        g_hash_table_insert(self->item_store, g_strdup(item_id), button);
}

/**
 * Backend has a new sidebar section for us
 */
static void brisk_classic_window_add_section(BriskMenuWindow *self, BriskSection *section,
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

        button = brisk_classic_category_button_new(section);
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(button),
                                    GTK_RADIO_BUTTON(self->section_box_leader));
        gtk_box_pack_start(GTK_BOX(box_target), button, FALSE, FALSE, 0);
        brisk_classic_window_associate_category(self, button);
        gtk_widget_show_all(button);

        /* Avoid new dupes */
        g_hash_table_insert(self->item_store, g_strdup(section_id), button);

        brisk_menu_window_select_sections(self);
}

/**
 * A backend needs us to invalidate the filters
 */
static void brisk_classic_window_invalidate_filter(BriskMenuWindow *self,
                                                   __brisk_unused__ BriskBackend *backend)
{
        gtk_list_box_invalidate_filter(GTK_LIST_BOX(BRISK_CLASSIC_WINDOW(self)->apps));
        gtk_list_box_invalidate_sort(GTK_LIST_BOX(BRISK_CLASSIC_WINDOW(self)->apps));
}

/**
 * A backend needs us to purge any data we have for it
 */
static void brisk_classic_window_reset(BriskMenuWindow *self, BriskBackend *backend)
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
        kids = gtk_container_get_children(GTK_CONTAINER(BRISK_CLASSIC_WINDOW(self)->apps));
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
static void brisk_classic_window_hide(GtkWidget *widget)
{
        BriskClassicWindow *self = NULL;
        GtkAdjustment *adjustment = NULL;

        /* Have parent deal with it first */
        GTK_WIDGET_CLASS(brisk_classic_window_parent_class)->hide(widget);

        self = BRISK_CLASSIC_WINDOW(widget);

        /* Remove search filter */
        gtk_entry_set_text(GTK_ENTRY(BRISK_MENU_WINDOW(self)->search), "");

        brisk_menu_window_select_sections(BRISK_MENU_WINDOW(self));

        /* Reset scrollbars */
        adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(self->apps_scroll));
        gtk_adjustment_set_value(adjustment, 0);
        adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(self->sidebar_scroll));
        gtk_adjustment_set_value(adjustment, 0);

        /* Unselect any current "apps" */
        gtk_list_box_select_row(GTK_LIST_BOX(self->apps), NULL);
}

/**
 * brisk_classic_window_class_init:
 *
 * Handle class initialisation
 */
static void brisk_classic_window_class_init(BriskClassicWindowClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        BriskMenuWindowClass *b_class = BRISK_MENU_WINDOW_CLASS(klazz);
        GtkWidgetClass *wid_class = GTK_WIDGET_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_classic_window_dispose;

        /* Window vtable hookup */
        b_class->get_id = brisk_classic_window_get_id;
        b_class->get_display_name = brisk_classic_window_get_display_name;
        b_class->update_screen_position = brisk_classic_window_update_screen_position;
        b_class->update_search = brisk_classic_window_update_search;
        b_class->add_item = brisk_classic_window_add_item;
        b_class->add_section = brisk_classic_window_add_section;
        b_class->invalidate_filter = brisk_classic_window_invalidate_filter;
        b_class->reset = brisk_classic_window_reset;

        /* widget vtable */
        wid_class->hide = brisk_classic_window_hide;
}

/**
 * brisk_classic_window_init:
 *
 * Handle construction of the BriskClassicWindow
 */
static void brisk_classic_window_init(BriskClassicWindow *self)
{
        GtkWidget *layout = NULL;
        GtkWidget *widget = NULL;
        GtkWidget *content = NULL;
        GtkWidget *scroll = NULL;
        GtkStyleContext *style = NULL;
        BriskMenuWindow *base;
        autofree(gchar) *txt_holder = NULL;
        autofree(gchar) *place_holder = NULL;

        brisk_classic_window_load_css(self);

        base = BRISK_MENU_WINDOW(self);

        style = gtk_widget_get_style_context(GTK_WIDGET(self));
        gtk_style_context_add_class(style, BRISK_STYLE_MAIN);

        brisk_menu_window_configure_grabs(base);

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
        /* Translators: This is the text shown in the "search box" with no content */
        txt_holder = g_strdup_printf("%s\u2026", _("Type to search"));
        gtk_entry_set_placeholder_text(GTK_ENTRY(widget), txt_holder);
        base->search = widget;
        g_signal_connect_swapped(widget, "changed", G_CALLBACK(brisk_menu_window_search), self);
        g_signal_connect_swapped(widget,
                                 "activate",
                                 G_CALLBACK(brisk_classic_window_key_activate),
                                 self);
        g_signal_connect(widget, "icon-press", G_CALLBACK(brisk_menu_window_clear_search), self);

        /* Content layout */
        content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
        gtk_box_pack_start(GTK_BOX(layout), content, TRUE, TRUE, 0);

        /* Sidebar for categories */
        widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        scroll = brisk_menu_sidebar_scroller_new();
        base->section_box_holder = widget;
        style = gtk_widget_get_style_context(base->section_box_holder);
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
                                 G_CALLBACK(brisk_classic_window_activated),
                                 self);

        /* Style up the app box */
        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, BRISK_STYLE_APPS_LIST);
        gtk_style_context_add_class(style, "view");
        gtk_style_context_add_class(style, "content-view");
        gtk_style_context_remove_class(style, "background");

        /* Translators: This message is shown when the search results are empty */
        place_holder = g_strdup_printf("<big>%s</big>", _("Sorry, no items found"));
        widget = gtk_label_new(place_holder);
        /* Add a placeholder when there are no apps for current search term */
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

        brisk_classic_window_setup_session_controls(self);

        gtk_window_set_default_size(GTK_WINDOW(self), 300, 510);

        /* Hook up keyboard events */
        g_signal_connect(self,
                         "key-release-event",
                         G_CALLBACK(brisk_menu_window_key_release),
                         NULL);
        g_signal_connect(self, "key-press-event", G_CALLBACK(brisk_menu_window_key_press), NULL);

        brisk_classic_window_build_sidebar(base);

        brisk_menu_window_init_backends(base);

        /* Hook up dbus later on */
        g_idle_add((GSourceFunc)brisk_menu_window_setup_session, self);

        gtk_widget_show_all(layout);
}

/*
 * brisk_classic_window_new:
 *
 * Return a newly created BriskClassicWindow
 */
BriskMenuWindow *brisk_classic_window_new(GtkWidget *relative_to)
{
        return g_object_new(BRISK_TYPE_CLASSIC_WINDOW,
                            "type",
                            GTK_WINDOW_POPUP,
                            "relative-to",
                            relative_to,
                            NULL);
}

/**
 * brisk_classic_window_associate_category:
 *
 * This will hook up the category button for events to enable us to filter the
 * list based on the active category.
 */
static void brisk_classic_window_associate_category(BriskMenuWindow *self, GtkWidget *button)
{
        g_signal_connect_swapped(button,
                                 "toggled",
                                 G_CALLBACK(brisk_classic_window_on_toggled),
                                 self);
        g_signal_connect_swapped(button,
                                 "enter-notify-event",
                                 G_CALLBACK(brisk_classic_window_on_enter),
                                 self);
}

/**
 * Fired by clicking a category button
 */
static void brisk_classic_window_on_toggled(BriskMenuWindow *self, GtkWidget *button)
{
        BriskClassicCategoryButton *cat = NULL;

        /* Skip a double signal due to using a group */
        if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button))) {
                return;
        }

        cat = BRISK_CLASSIC_CATEGORY_BUTTON(button);
        g_object_get(cat, "section", &self->active_section, NULL);

        /* Start the filter. */
        brisk_classic_window_invalidate_filter(self, NULL);
}

/**
 * Fired by entering into the category button with a roll over
 */
static gboolean brisk_classic_window_on_enter(BriskMenuWindow *self,
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
static void brisk_classic_window_load_css(BriskClassicWindow *self)
{
        GtkCssProvider *css = NULL;
        autofree(GFile) *file = NULL;
        autofree(GError) *err = NULL;
        GdkScreen *screen = NULL;

        file = g_file_new_for_uri("resource://com/solus-project/brisk/menu/classic/styling.css");
        if (!file) {
                return;
        }

        css = gtk_css_provider_new();
        self->css = css;
        screen = gtk_widget_get_screen(GTK_WIDGET(self));
        gtk_style_context_add_provider_for_screen(screen,
                                                  GTK_STYLE_PROVIDER(css),
                                                  GTK_STYLE_PROVIDER_PRIORITY_FALLBACK);

        if (!gtk_css_provider_load_from_file(css, file, &err)) {
                g_warning("Failed to load CSS: %s\n", err->message);
                return;
        }
}

static void brisk_classic_window_key_activate(BriskClassicWindow *self, __brisk_unused__ gpointer v)
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

static void brisk_classic_window_activated(__brisk_unused__ BriskMenuWindow *self,
                                           GtkListBoxRow *row, __brisk_unused__ gpointer v)
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
 * Create the graphical buttons for session control
 */
static void brisk_classic_window_setup_session_controls(BriskClassicWindow *self)
{
        GtkWidget *widget = NULL;
        GtkWidget *box = NULL;
        GtkStyleContext *style = NULL;

        box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_margin_bottom(box, 4);

        gtk_box_pack_end(GTK_BOX(self->sidebar_wrap), box, FALSE, FALSE, 0);
        gtk_widget_set_halign(box, GTK_ALIGN_CENTER);

        /* Add a separator for visual consistency */
        widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_end(GTK_BOX(self->sidebar_wrap), widget, FALSE, FALSE, 3);

        /* Logout */
        widget = gtk_button_new_from_icon_name("brisk_system-log-out-symbolic", GTK_ICON_SIZE_MENU);
        self->button_logout = widget;
        g_signal_connect_swapped(widget, "clicked", G_CALLBACK(brisk_menu_window_logout), self);
        gtk_widget_set_tooltip_text(widget, _("End the current session"));
        gtk_widget_set_can_focus(widget, FALSE);
        gtk_container_add(GTK_CONTAINER(box), widget);
        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_FLAT);

        /* Lock */
        widget = gtk_button_new_from_icon_name("system-lock-screen-symbolic",
                                               GTK_ICON_SIZE_SMALL_TOOLBAR);
        self->button_lock = widget;
        g_signal_connect_swapped(widget, "clicked", G_CALLBACK(brisk_menu_window_lock), self);
        gtk_widget_set_tooltip_text(widget, _("Lock the screen"));
        gtk_widget_set_can_focus(widget, FALSE);
        gtk_container_add(GTK_CONTAINER(box), widget);
        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_FLAT);

        /* Shutdown */
        widget =
            gtk_button_new_from_icon_name("system-shutdown-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
        self->button_shutdown = widget;
        g_signal_connect_swapped(widget, "clicked", G_CALLBACK(brisk_menu_window_shutdown), self);
        gtk_widget_set_tooltip_text(widget, _("Turn off the device"));
        gtk_widget_set_can_focus(widget, FALSE);
        gtk_container_add(GTK_CONTAINER(box), widget);
        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_FLAT);
}

/**
 * Begin a build of the menu structure
 */
static void brisk_classic_window_build_sidebar(BriskMenuWindow *self)
{
        GtkWidget *sep = NULL;
        autofree(gstrv) *shortcuts = NULL;

        brisk_classic_window_set_filters_enabled(BRISK_CLASSIC_WINDOW(self), FALSE);

        /* Special leader to control group association, hidden from view */
        self->section_box_leader = gtk_radio_button_new(NULL);
        gtk_box_pack_start(GTK_BOX(self->section_box_holder),
                           self->section_box_leader,
                           FALSE,
                           FALSE,
                           0);
        gtk_widget_set_no_show_all(self->section_box_leader, TRUE);
        gtk_widget_hide(self->section_box_leader);

        /* Separate the things */
        sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_start(GTK_BOX(self->section_box_holder), sep, FALSE, FALSE, 5);
        gtk_widget_show_all(sep);

        /* Load the shortcuts up */
        shortcuts = g_settings_get_strv(self->settings, "pinned-shortcuts");
        if (!shortcuts) {
                return;
        }

        /* Add from gsettings */
        for (guint i = 0; i < g_strv_length(shortcuts); i++) {
                brisk_classic_window_add_shortcut(self, shortcuts[i]);
        }

        brisk_classic_window_set_filters_enabled(BRISK_CLASSIC_WINDOW(self), TRUE);
}

/**
 * brisk_classic_window_add_shortcut
 *
 * If we can create a .desktop launcher for the given name, add a new button to
 * the sidebar as a quick launch facility.
 */
static void brisk_classic_window_add_shortcut(BriskMenuWindow *self, const gchar *id)
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
        gtk_box_pack_start(GTK_BOX(self->section_box_holder), button, FALSE, FALSE, 1);
}

/**
 * Enable or disable the filters between building of the menus
 */
static void brisk_classic_window_set_filters_enabled(BriskClassicWindow *self, gboolean enabled)
{
        BRISK_MENU_WINDOW(self)->filtering = enabled;
        if (enabled) {
                gtk_list_box_set_filter_func(GTK_LIST_BOX(self->apps),
                                             brisk_classic_window_filter_apps,
                                             self,
                                             NULL);
                gtk_list_box_set_sort_func(GTK_LIST_BOX(self->apps),
                                           brisk_classic_window_sort,
                                           self,
                                           NULL);
                return;
        }
        gtk_list_box_set_filter_func(GTK_LIST_BOX(self->apps), NULL, NULL, NULL);
        gtk_list_box_set_sort_func(GTK_LIST_BOX(self->apps), NULL, NULL, NULL);
}

/**
 * brisk_classic_window_filter_apps:
 *
 * Responsible for filtering the selection based on active group or search
 * term.
 */
__brisk_pure__ static gboolean brisk_classic_window_filter_apps(GtkListBoxRow *row, gpointer v)
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

static gint brisk_classic_window_sort(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer v)
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
