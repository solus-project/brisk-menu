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

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "menu-private.h"
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

void brisk_menu_window_setup_session(BriskMenuWindow *self)
{
        GtkWidget *widget = NULL;
        GtkWidget *box = NULL;
        GtkStyleContext *style = NULL;

        box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_margin_bottom(box, 4);
        style = gtk_widget_get_style_context(box);
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_LINKED);

        gtk_box_pack_end(GTK_BOX(self->sidebar_wrap), box, FALSE, FALSE, 0);
        gtk_widget_set_halign(box, GTK_ALIGN_CENTER);

        /* Logout.. TODO: Find a better icon! */
        widget = gtk_button_new_from_icon_name("application-exit-symbolic", GTK_ICON_SIZE_MENU);
        gtk_widget_set_tooltip_text(widget, "End the current session");
        gtk_container_add(GTK_CONTAINER(box), widget);

        /* Lock */
        widget = gtk_button_new_from_icon_name("system-lock-screen-symbolic",
                                               GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_widget_set_tooltip_text(widget, "Lock the screen");
        gtk_container_add(GTK_CONTAINER(box), widget);

        /* Shutdown */
        widget =
            gtk_button_new_from_icon_name("system-shutdown-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
        gtk_widget_set_tooltip_text(widget, "Turn off the device");
        gtk_container_add(GTK_CONTAINER(box), widget);
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
