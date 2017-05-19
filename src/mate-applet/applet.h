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

#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BriskMenuApplet BriskMenuApplet;
typedef struct _BriskMenuAppletClass BriskMenuAppletClass;

#define BRISK_TYPE_MENU_APPLET brisk_menu_applet_get_type()
#define BRISK_MENU_APPLET(o)                                                                       \
        (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_MENU_APPLET, BriskMenuApplet))
#define BRISK_IS_MENU_APPLET(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_MENU_APPLET))
#define BRISK_MENU_APPLET_CLASS(o)                                                                 \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_MENU_APPLET, BriskMenuAppletClass))
#define BRISK_IS_MENU_APPLET_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_MENU_APPLET))
#define BRISK_MENU_APPLET_GET_CLASS(o)                                                             \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_MENU_APPLET, BriskMenuAppletClass))

GType brisk_menu_applet_get_type(void);

void brisk_menu_applet_edit_menus(GtkAction *action, BriskMenuApplet *applet);

G_END_DECLS

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
