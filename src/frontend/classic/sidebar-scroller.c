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
#include "sidebar-scroller.h"
#include <gtk/gtk.h>

BRISK_END_PEDANTIC

struct _BriskMenuSidebarScrollerClass {
        GtkScrolledWindowClass parent_class;
};

struct _BriskMenuSidebarScroller {
        GtkScrolledWindow parent;
};

G_DEFINE_TYPE(BriskMenuSidebarScroller, brisk_menu_sidebar_scroller, GTK_TYPE_SCROLLED_WINDOW)

static void brisk_menu_sidebar_scroller_get_preferred_height(GtkWidget *widget, gint *min_height,
                                                             gint *nat_height);

/**
 * brisk_menu_sidebar_scroller_new:
 *
 * Construct a new BriskMenuSidebarScroller object
 */
GtkWidget *brisk_menu_sidebar_scroller_new()
{
        return g_object_new(BRISK_TYPE_MENU_SIDEBAR_SCROLLER,
                            "hadjustment",
                            NULL,
                            "vadjustment",
                            NULL,
                            NULL);
}

/**
 * brisk_menu_sidebar_scroller_class_init:
 *
 * Handle class initialisation
 */
static void brisk_menu_sidebar_scroller_class_init(BriskMenuSidebarScrollerClass *klazz)
{
        GtkWidgetClass *wid_class = GTK_WIDGET_CLASS(klazz);
        wid_class->get_preferred_height = brisk_menu_sidebar_scroller_get_preferred_height;
}

/**
 * brisk_menu_sidebar_scroller_init:
 *
 * Handle construction of the BriskMenuEntryButton
 */
static void brisk_menu_sidebar_scroller_init(BriskMenuSidebarScroller *self)
{
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(self),
                                       GTK_POLICY_NEVER,
                                       GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_overlay_scrolling(GTK_SCROLLED_WINDOW(self), FALSE);
}

static void brisk_menu_sidebar_scroller_get_preferred_height(GtkWidget *widget, gint *min_height,
                                                             gint *nat_height)
{
        GdkScreen *screen = NULL;
        GdkWindow *window = NULL;
        GdkRectangle geom = { 0 };
        gint applet_x, applet_y = 0;
        gint mon = 0;

        /* Find out where we are on screen */
        window = gtk_widget_get_window(widget);
        gdk_window_get_origin(window, &applet_x, &applet_y);

        /* Grab the geometry for the monitor we're currently on */
        screen = gtk_widget_get_screen(widget);
        mon = gdk_screen_get_monitor_at_point(screen, applet_x, applet_y);
        gdk_screen_get_monitor_geometry(screen, mon, &geom);

        gint max_height = geom.height - 200;

        GtkBin *bin = NULL;
        GtkWidget *child = NULL;

        bin = GTK_BIN(widget);
        child = gtk_bin_get_child(bin);

        if (child) {
                gtk_widget_get_preferred_height(child, min_height, nat_height);
                *min_height = MIN(max_height, *min_height);
                *nat_height = MIN(max_height, *nat_height);
        } else {
                *min_height = *nat_height = 0;
        }
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
