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

#define _GNU_SOURCE

#include "config.h"
#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "applet.h"
#include "brisk-resources.h"
#include "key-binder.h"
#include "menu-private.h"
#include "menu-window.h"
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
        BriskKeyBinder *binder;
};

G_DEFINE_TYPE(BriskMenuApplet, brisk_menu_applet, PANEL_TYPE_APPLET)

/**
 * We have no .ctor in the .a file - so it doesn't link
 */
__attribute__((constructor)) static void brisk_resource_init(void)
{
        brisk_resources_register_resource();
}

/**
 * Again, no .dtor due to link issues, so we do it here
 */
__attribute__((destructor)) static void brisk_resource_deinit(void)
{
        brisk_resources_unregister_resource();
}

/**
 * Handle showing of the menu
 */
static gboolean button_press_cb(BriskMenuApplet *self, GdkEvent *event, gpointer v);
static void hotkey_cb(GdkEvent *event, gpointer v);

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

        /* gobject vtable hookup */
        obj_class->dispose = brisk_menu_applet_dispose;
}

/**
 * brisk_menu_applet_init:
 *
 * Handle construction of the BriskMenuApplet
 */
static void brisk_menu_applet_init(BriskMenuApplet *self)
{
        GtkWidget *toggle, *layout, *image, *label, *menu = NULL;

        self->binder = brisk_key_binder_new();

        /* Create the toggle button */
        toggle = gtk_toggle_button_new();
        self->toggle = toggle;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), FALSE);
        gtk_container_add(GTK_CONTAINER(self), toggle);
        g_signal_connect_swapped(toggle, "button-press-event", G_CALLBACK(button_press_cb), self);
        gtk_button_set_relief(GTK_BUTTON(toggle), GTK_RELIEF_NONE);

        /* Layout will contain icon + label */
        layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_container_add(GTK_CONTAINER(toggle), layout);

        /* Image appears first always */
        image = gtk_image_new_from_icon_name("start-here-symbolic", GTK_ICON_SIZE_MENU);
        self->image = image;
        gtk_box_pack_start(GTK_BOX(layout), image, FALSE, FALSE, 0);
        gtk_widget_set_margin_end(image, 4);
        gtk_widget_set_halign(image, GTK_ALIGN_START);

        /* Now add the label */
        label = gtk_label_new(_("Menu"));
        self->label = label;
        gtk_box_pack_start(GTK_BOX(layout), label, TRUE, TRUE, 0);
        gtk_widget_set_margin_end(label, 4);

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

        /* Render "active" toggle only when the window is open, automatically. */
        g_object_bind_property(menu, "visible", toggle, "active", G_BINDING_DEFAULT);

        /* Load initially in the idle loop, prevent lagging panel on startup */
        g_idle_add((GSourceFunc)brisk_menu_window_load_menus, self->menu);

        if (!brisk_key_binder_bind(self->binder, "<Ctrl>F10", hotkey_cb, self)) {
                g_message("Failed to bind keyboard shortcut");
        }

        brisk_menu_window_set_search_position(BRISK_MENU_WINDOW(self->menu), BRISK_SEARCH_POS_TOP);
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

static gboolean brisk_menu_applet_factory(MatePanelApplet *applet, const gchar *id,
                                          __brisk_unused__ gpointer v)
{
        if (!g_str_has_prefix(id, "BriskMenu")) {
                return FALSE;
        }
        const char *home = NULL;
        __attribute__((unused)) int ret = 0;

        home = g_get_home_dir();
        if (home) {
                ret = chdir(home);
        }

        g_set_application_name(_("Brisk Menu Launcher"));
        gtk_widget_show(GTK_WIDGET(applet));
        return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("BriskMenuFactory", BRISK_TYPE_MENU_APPLET, "BriskMenu",
                                      brisk_menu_applet_factory, NULL)

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
