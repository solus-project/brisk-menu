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

#include "config.h"
#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "applet.h"
#include "frontend/menu-private.h"
#include "frontend/menu-window.h"
#include "lib/key-binder.h"
#include <gio/gdesktopappinfo.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <mate-panel-applet.h>
BRISK_END_PEDANTIC

struct _BriskMenuAppletClass {
        MatePanelAppletClass parent_class;
};

struct _BriskMenuApplet {
        MatePanelApplet parent;
        GtkWidget *toggle;
        GtkWidget *label;
        GtkWidget *image;
        GtkWidget *menu;
        GSettings *settings;
        gchar *shortcut;
        BriskKeyBinder *binder;
};

G_DEFINE_TYPE(BriskMenuApplet, brisk_menu_applet, PANEL_TYPE_APPLET)

static gint icon_sizes[] = { 16, 24, 32, 48, 64, 96, 128, 256 };

/**
 * Handle showing of the menu
 */
static gboolean button_press_cb(BriskMenuApplet *self, GdkEvent *event, gpointer v);
static void hotkey_cb(GdkEvent *event, gpointer v);
static void brisk_menu_applet_change_orient(MatePanelApplet *applet, MatePanelAppletOrient orient);
static void brisk_menu_applet_change_size(MatePanelApplet *applet, guint size);
static void brisk_menu_adapt_layout(MatePanelApplet *applet, MatePanelAppletOrient orient);

/* Handle applet settings */
void brisk_menu_applet_init_settings(BriskMenuApplet *self);
static void brisk_menu_applet_settings_changed(GSettings *settings, const gchar *key, gpointer v);
void brisk_menu_applet_update_hotkey(BriskMenuApplet *self, gchar *key);

/**
 * Update the position for the menu.
 */
static void place_menu(BriskMenuApplet *self)
{
        GdkScreen *screen = NULL;
        GtkAllocation alloc = { 0 };
        GdkWindow *window = NULL;
        GdkRectangle geom = { 0 };
        gint rx, ry = 0;
        gint ww, wh = 0;
        gint mon = 0;
        gint tx, ty = 0;

        gtk_widget_get_allocation(GTK_WIDGET(self), &alloc);
        gtk_window_get_size(GTK_WINDOW(self->menu), &ww, &wh);

        if (!gtk_widget_get_realized(GTK_WIDGET(self))) {
                gtk_widget_realize(GTK_WIDGET(self));
        }
        window = gtk_widget_get_window(GTK_WIDGET(self));
        gdk_window_get_origin(window, &rx, &ry);

        screen = gtk_widget_get_screen(GTK_WIDGET(self));
        mon = gdk_screen_get_monitor_at_point(screen, rx, ry);
        gdk_screen_get_monitor_geometry(screen, mon, &geom);

        /** We must be at the bottom of the screen. One hopes. */
        if (ry + wh > geom.y + geom.height) {
                ty = (geom.y + geom.height) - (alloc.height + wh);
        } else {
                /* Go to the bottom */
                ty = ry + alloc.height;
        }

        tx = rx;
        /* Bound the right side */
        if (tx + ww > (geom.x + geom.width)) {
                tx = (geom.x + geom.width) - (ww);
        }
        /* Bound the left side */
        if (tx < geom.x) {
                tx = geom.x;
        }
        gtk_window_move(GTK_WINDOW(self->menu), tx, ty);
}

/**
 * Handle hiding the menu when it comes to the shortcut key only.
 * i.e. the Super_L key.
 */
static gboolean brisk_menu_applet_key_press(BriskMenuApplet *self, GdkEvent *event, GtkWidget *menu)
{
        autofree(gchar) *accel_name = NULL;

        if (!self->shortcut) {
                return GDK_EVENT_PROPAGATE;
        }

        accel_name = gtk_accelerator_name(event->key.keyval, event->key.state);
        if (!accel_name || g_ascii_strcasecmp(self->shortcut, accel_name) != 0) {
                return GDK_EVENT_PROPAGATE;
        }

        gtk_widget_hide(menu);
        return GDK_EVENT_STOP;
}

/**
 * brisk_menu_applet_dispose:
 *
 * Clean up a BriskMenuApplet instance
 */
static void brisk_menu_applet_dispose(GObject *obj)
{
        BriskMenuApplet *self = NULL;

        self = BRISK_MENU_APPLET(obj);

        /* Tear down the menu */
        if (self->menu) {
                gtk_widget_hide(self->menu);
                g_clear_pointer(&self->menu, gtk_widget_destroy);
        }

        g_clear_object(&self->binder);
        g_clear_object(&self->settings);
        g_clear_pointer(&self->shortcut, g_free);

        G_OBJECT_CLASS(brisk_menu_applet_parent_class)->dispose(obj);
}

/**
 * brisk_menu_applet_class_init:
 *
 * Handle class initialisation
 */
static void brisk_menu_applet_class_init(BriskMenuAppletClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        MatePanelAppletClass *mate_class = MATE_PANEL_APPLET_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_menu_applet_dispose;

        /* mate vtable hookup */
        mate_class->change_orient = brisk_menu_applet_change_orient;
        mate_class->change_size = brisk_menu_applet_change_size;
}

void brisk_menu_applet_init_settings(BriskMenuApplet *self)
{
        self->settings = g_settings_new("com.solus-project.brisk-menu");

        /* capture changes in settings that affect the menu applet */
        g_signal_connect(self->settings,
                         "changed",
                         G_CALLBACK(brisk_menu_applet_settings_changed),
                         self);

        /* Pump applet settings */
        brisk_menu_applet_settings_changed(self->settings, "hot-key", self);
}

/**
 * brisk_menu_applet_init:
 *
 * Handle construction of the BriskMenuApplet
 */
static void brisk_menu_applet_init(BriskMenuApplet *self)
{
        GtkWidget *toggle, *layout, *image, *label, *menu = NULL;
        GtkStyleContext *style = NULL;

        self->binder = brisk_key_binder_new();
        brisk_menu_applet_init_settings(self);

        /* Create the toggle button */
        toggle = gtk_toggle_button_new();
        self->toggle = toggle;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), FALSE);
        gtk_container_add(GTK_CONTAINER(self), toggle);
        g_signal_connect_swapped(toggle, "button-press-event", G_CALLBACK(button_press_cb), self);
        gtk_button_set_relief(GTK_BUTTON(toggle), GTK_RELIEF_NONE);
        style = gtk_widget_get_style_context(toggle);
        gtk_style_context_add_class(style, BRISK_STYLE_BUTTON);

        /* Layout will contain icon + label */
        layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_halign(layout, GTK_ALIGN_CENTER);
        gtk_container_add(GTK_CONTAINER(toggle), layout);

        /* Image appears first always */
        image = gtk_image_new_from_icon_name("start-here-symbolic", GTK_ICON_SIZE_MENU);
        self->image = image;
        gtk_box_pack_start(GTK_BOX(layout), image, FALSE, FALSE, 0);
        gtk_widget_set_margin_end(image, 4);
        gtk_widget_set_halign(image, GTK_ALIGN_START);

        /* Now add the label */
        label = gtk_label_new(NULL);
        self->label = label;
        gtk_box_pack_start(GTK_BOX(layout), label, TRUE, TRUE, 0);
        gtk_widget_set_margin_end(label, 4);
        /* Set it up for visibility toggling */
        gtk_widget_show_all(label);
        gtk_widget_set_no_show_all(label, TRUE);
        gtk_widget_hide(label);

        /* Update label visibility dependent on config */
        g_settings_bind(self->settings, "label-visible", label, "visible", G_SETTINGS_BIND_GET);

        /* Pump the label setting */
        brisk_menu_applet_settings_changed(self->settings, "label-text", self);

        /* Fix label alignment */
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f);
        G_GNUC_END_IGNORE_DEPRECATIONS

        /* Applet hookup */
        mate_panel_applet_set_flags(MATE_PANEL_APPLET(self), MATE_PANEL_APPLET_EXPAND_MINOR);
        mate_panel_applet_set_background_widget(MATE_PANEL_APPLET(self), GTK_WIDGET(self));

        /* Now show all content */
        gtk_widget_show_all(toggle);

        /* Construct our menu */
        menu = brisk_menu_window_new();
        self->menu = menu;
        /* We handle the shortcut side of the events, not the escape key */
        g_signal_connect_swapped(self->menu,
                                 "key-press-event",
                                 G_CALLBACK(brisk_menu_applet_key_press),
                                 self);

        /* Render "active" toggle only when the window is open, automatically. */
        g_object_bind_property(menu, "visible", toggle, "active", G_BINDING_DEFAULT);

        /* Load initially in the idle loop, prevent lagging panel on startup */
        g_idle_add((GSourceFunc)brisk_menu_window_load_menus, self->menu);

        /* Fix the orient now we're up */
        brisk_menu_window_set_orient(BRISK_MENU_WINDOW(self->menu),
                                     mate_panel_applet_get_orient(MATE_PANEL_APPLET(self)));

        brisk_menu_adapt_layout(MATE_PANEL_APPLET(self),
                                mate_panel_applet_get_orient(MATE_PANEL_APPLET(self)));

        /* Pump the settings */
        brisk_menu_window_pump_settings(BRISK_MENU_WINDOW(self->menu));
}

/**
 * Toggle the menu visibility on a button press
 */
static gboolean button_press_cb(BriskMenuApplet *self, GdkEvent *event, __brisk_unused__ gpointer v)
{
        if (event->button.button != 1) {
                return GDK_EVENT_PROPAGATE;
        }

        gboolean vis = !gtk_widget_get_visible(self->menu);
        if (vis) {
                place_menu(self);
        }

        gtk_widget_set_visible(self->menu, vis);

        return GDK_EVENT_STOP;
}

/**
 * Called in idle once back out of the event
 */
static gboolean toggle_menu(BriskMenuApplet *self)
{
        gboolean vis = !gtk_widget_get_visible(self->menu);
        if (vis) {
                place_menu(self);
        }

        gtk_widget_set_visible(self->menu, vis);
        return FALSE;
}

/**
 * Handle global hotkey press
 */
static void hotkey_cb(__brisk_unused__ GdkEvent *event, gpointer v)
{
        g_idle_add((GSourceFunc)toggle_menu, v);
}

/**
 * Callback for changing applet settings
 */
static void brisk_menu_applet_settings_changed(GSettings *settings, const gchar *key, gpointer v)
{
        BriskMenuApplet *self = v;
        autofree(gchar) *value = NULL;

        if (g_str_equal(key, "hot-key")) {
                value = g_settings_get_string(settings, key);
                brisk_menu_applet_update_hotkey(self, value);
        } else if (g_str_equal(key, "label-text")) {
                value = g_settings_get_string(settings, key);
                if (g_str_equal(value, "")) {
                        gtk_label_set_text(GTK_LABEL(self->label), _("Menu"));
                } else {
                        gtk_label_set_text(GTK_LABEL(self->label), value);
                }
        }
}

/**
 * Update the applet hotkey in accordance with settings
 */
void brisk_menu_applet_update_hotkey(BriskMenuApplet *self, gchar *key)
{
        if (self->shortcut) {
                brisk_key_binder_unbind(self->binder, self->shortcut);
                g_clear_pointer(&self->shortcut, g_free);
        }

        if (!brisk_key_binder_bind(self->binder, key, hotkey_cb, self)) {
                g_message("Failed to bind keyboard shortcut");
                return;
        }

        self->shortcut = g_strdup(key);
}

/**
 * Panel orientation changed, tell the menu
 */
static void brisk_menu_applet_change_orient(MatePanelApplet *applet, MatePanelAppletOrient orient)
{
        BriskMenuApplet *self = BRISK_MENU_APPLET(applet);

        brisk_menu_window_set_orient(BRISK_MENU_WINDOW(self->menu), orient);
        brisk_menu_adapt_layout(applet, orient);
}

static void brisk_menu_applet_change_size(MatePanelApplet *applet, guint size)
{
        BriskMenuApplet *self = BRISK_MENU_APPLET(applet);

        gint final_size = icon_sizes[0];

        for (guint i = 0; i < G_N_ELEMENTS(icon_sizes); i++) {
                if (icon_sizes[i] > (gint)size - 2) {
                        break;
                }
                final_size = icon_sizes[i];
        }

        gtk_image_set_pixel_size(GTK_IMAGE(self->image), final_size);
}

static void brisk_menu_adapt_layout(MatePanelApplet *applet, MatePanelAppletOrient orient)
{
        BriskMenuApplet *self = BRISK_MENU_APPLET(applet);
        GtkStyleContext *style = NULL;

        style = gtk_widget_get_style_context(self->toggle);

        if (orient == MATE_PANEL_APPLET_ORIENT_LEFT || orient == MATE_PANEL_APPLET_ORIENT_RIGHT) {
                gtk_widget_hide(self->label);
                gtk_widget_set_halign(self->image, GTK_ALIGN_CENTER);
                gtk_style_context_add_class(style, BRISK_STYLE_BUTTON_VERTICAL);
                gtk_widget_set_margin_end(self->image, 0);
                return;
        }

        if (g_settings_get_boolean(self->settings, "label-visible")) {
                gtk_widget_show(GTK_WIDGET(self->label));
        }
        gtk_widget_set_halign(GTK_WIDGET(self->image), GTK_ALIGN_START);
        gtk_style_context_remove_class(style, BRISK_STYLE_BUTTON_VERTICAL);
        gtk_widget_set_margin_end(self->image, 4);
}

void brisk_menu_applet_edit_menus(__brisk_unused__ GtkAction *action, BriskMenuApplet *self)
{
        static const char *editors[] = {
                "menulibre.desktop", "mozo.desktop",
        };
        static const char *binaries[] = {
                "menulibre", "mozo",
        };
        for (size_t i = 0; i < G_N_ELEMENTS(editors); i++) {
                autofree(gchar) *p = NULL;
                autofree(GAppInfo) *app = NULL;
                BriskMenuLauncher *launcher = ((BRISK_MENU_WINDOW(self->menu))->launcher);
                GDesktopAppInfo *info = NULL;

                p = g_find_program_in_path(binaries[i]);
                if (!p) {
                        continue;
                }

                info = g_desktop_app_info_new(editors[i]);
                if (!info) {
                        app = g_app_info_create_from_commandline(p,
                                                                 NULL,
                                                                 G_APP_INFO_CREATE_NONE,
                                                                 NULL);
                } else {
                        app = G_APP_INFO(info);
                }
                if (!app) {
                        continue;
                }
                info = G_DESKTOP_APP_INFO(app);
                brisk_menu_launcher_start(launcher, GTK_WIDGET(self), app);
                return;
        }
        g_message("Failed to launch menu editor");
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
