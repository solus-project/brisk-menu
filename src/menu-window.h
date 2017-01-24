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

#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

/**
 * Defines the position of the Brisk menu search bar
 */
typedef enum {
        BRISK_SEARCH_POS_TOP = 0,   /* Show at top of window */
        BRISK_SEARCH_POS_BOTTOM = 1 /* Show at bottom of window */
} BriskSearchPosition;

G_BEGIN_DECLS

typedef struct _BriskMenuWindow BriskMenuWindow;
typedef struct _BriskMenuWindowClass BriskMenuWindowClass;

#define BRISK_TYPE_MENU_WINDOW brisk_menu_window_get_type()
#define BRISK_MENU_WINDOW(o)                                                                       \
        (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_MENU_WINDOW, BriskMenuWindow))
#define BRISK_IS_MENU_WINDOW(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_MENU_WINDOW))
#define BRISK_MENU_WINDOW_CLASS(o)                                                                 \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_MENU_WINDOW, BriskMenuWindowClass))
#define BRISK_IS_MENU_WINDOW_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_MENU_WINDOW))
#define BRISK_MENU_WINDOW_GET_CLASS(o)                                                             \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_MENU_WINDOW, BriskMenuWindowClass))

GtkWidget *brisk_menu_window_new(void);

GType brisk_menu_window_get_type(void);

void brisk_menu_window_set_search_position(BriskMenuWindow *window, BriskSearchPosition position);

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
