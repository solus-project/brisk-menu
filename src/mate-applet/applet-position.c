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
 * brisk_menu_applet_adapt_layout:
 *
 * Update our layout in response to an orientation change.
 * Primarily we're hiding our label automatically here and maximizing the space
 * available to the icon.
 */
void brisk_menu_applet_adapt_layout(MatePanelApplet *applet, MatePanelAppletOrient orient)
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
