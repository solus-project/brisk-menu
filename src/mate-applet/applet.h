/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016-2018 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>
#include <mate-panel-applet.h>

G_BEGIN_DECLS

typedef struct _BriskMenuApplet BriskMenuApplet;
typedef struct _BriskMenuAppletClass BriskMenuAppletClass;

typedef enum {
        WINDOW_TYPE_MIN = 0,
        WINDOW_TYPE_CLASSIC = 1 << 0,
        WINDOW_TYPE_DASH = 1 << 1,
        WINDOW_TYPE_AUTOMATIC = 1 << 2,
        WINDOW_TYPE_MAX,
} WindowType;

struct _BriskMenuAppletClass {
        MatePanelAppletClass parent_class;
};

/**
 * BriskMenuApplet is the passive portion of the Brisk Menu, in that it
 * provides merely an access path to the main Brisk Menu Window itself.
 */
struct _BriskMenuApplet {
        MatePanelApplet parent;

        GtkWidget *toggle;            /**<Main display button, bulk of the applet */
        GtkWidget *label;             /**<Display label "Menu" */
        GtkWidget *image;             /**<Icon display beside label */
        GtkWidget *menu;              /**<BriskMenuWindow instance */
        GSettings *settings;          /**<Our settings store */
        MatePanelAppletOrient orient; /**<Current position for the panel */
};

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
void brisk_menu_applet_show_about(GtkAction *action, BriskMenuApplet *applet);

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
