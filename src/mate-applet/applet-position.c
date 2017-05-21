/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016-2017 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "applet.h"
#include "lib/styles.h"
BRISK_END_PEDANTIC

/**
 * brisk_menu_applet_update_position:
 *
 * Update the posititon of the menu based on the current orientation
 * and position of the applet.
 *
 * TODO: Add support for vertical layouts, and make this code generally
 * Less Crumby.
 */
void brisk_menu_applet_update_position(BriskMenuApplet *self)
{
        GdkScreen *screen = NULL;
        GtkAllocation alloc = { 0 };
        GdkWindow *window = NULL;
        GdkRectangle geom = { 0 };
        gint applet_x, applet_y = 0;          /* Real X, Y of this applet, on screen */
        gint window_width, window_height = 0; /* Window width & height */
        gint mon = 0;                         /* Monitor to display on */
        gint window_x, window_y = 0;          /* Target X, Y */

        /* Forcibly realize ourselves */
        if (!gtk_widget_get_realized(GTK_WIDGET(self))) {
                gtk_widget_realize(GTK_WIDGET(self));
        }

        /* Forcibly realize the window */
        if (!gtk_widget_get_realized(self->menu)) {
                gtk_widget_realize(self->menu);
        }

        gtk_widget_get_allocation(GTK_WIDGET(self), &alloc);

        /* Find out where we are on screen */
        window = gtk_widget_get_window(GTK_WIDGET(self));
        gdk_window_get_origin(window, &applet_x, &applet_y);

        /* Find out the window size */
        gtk_window_get_size(GTK_WINDOW(self->menu), &window_width, &window_height);

        /* Grab the geometry for the monitor we're currently on */
        screen = gtk_widget_get_screen(GTK_WIDGET(self));
        mon = gdk_screen_get_monitor_at_point(screen, applet_x, applet_y);
        gdk_screen_get_monitor_geometry(screen, mon, &geom);

        switch (self->orient) {
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
                /* Left vertical panel, appear to the RHS of it */
                window_x = applet_x + alloc.width;
                window_y = applet_y;
                break;
        case MATE_PANEL_APPLET_ORIENT_LEFT:
                /* Right vertical panel, appear to the LHS of it */
                window_x = applet_x - window_width;
                window_y = applet_y;
                break;
        case MATE_PANEL_APPLET_ORIENT_DOWN:
                /* Top panel, appear below it */
                window_x = applet_x;
                window_y = applet_y + alloc.height;
                break;
        case MATE_PANEL_APPLET_ORIENT_UP:
        default:
                /* Bottom panel, appear above it */
                window_x = applet_x;
                window_y = applet_y - window_height;
                break;
        }

        /* Bound the right side */
        if (window_x + window_width > (geom.x + geom.width)) {
                window_x = (geom.x + geom.width) - window_width;
                if (self->orient == MATE_PANEL_APPLET_ORIENT_LEFT) {
                        window_x -= alloc.width;
                }
        }

        /* Bound the left side */
        if (window_x < geom.x) {
                window_x = geom.x;
                if (self->orient == MATE_PANEL_APPLET_ORIENT_RIGHT) {
                        window_x -= alloc.width;
                }
        }

        gtk_window_move(GTK_WINDOW(self->menu), window_x, window_y);
}

/**
 * brisk_menu_applet_adapt_layout:
 *
 * Update our layout in response to an orientation change.
 * Primarily we're hiding our label automatically here and maximizing the space
 * available to the icon.
 */
void brisk_menu_applet_adapt_layout(BriskMenuApplet *self)
{
        GtkStyleContext *style = NULL;

        style = gtk_widget_get_style_context(self->toggle);

        switch (self->orient) {
        case MATE_PANEL_APPLET_ORIENT_LEFT:
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
                /* Handle vertical panel layout */
                gtk_widget_hide(self->label);
                gtk_widget_set_halign(self->image, GTK_ALIGN_CENTER);
                gtk_style_context_add_class(style, BRISK_STYLE_BUTTON_VERTICAL);
                gtk_widget_set_margin_end(self->image, 0);
                break;
        default:
                /* We're a horizontal panel */
                gtk_widget_set_visible(self->label,
                                       g_settings_get_boolean(self->settings, "label-visible"));
                gtk_widget_set_halign(self->image, GTK_ALIGN_START);
                gtk_style_context_remove_class(style, BRISK_STYLE_BUTTON_VERTICAL);
                gtk_widget_set_margin_end(self->image, 4);
                break;
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
