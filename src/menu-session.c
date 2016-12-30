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
        GtkWidget *button = NULL;
        GtkWidget *box = NULL;

        box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_button_box_set_layout(GTK_BUTTON_BOX(box), GTK_BUTTONBOX_CENTER);

        gtk_box_pack_end(GTK_BOX(self->sidebar_wrap), box);

        widget = gtk_button_new_from_icon_name("system-log-out", GTK_ICON_SIZE_BUTTON);
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
