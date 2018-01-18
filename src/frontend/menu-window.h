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

#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

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

GType brisk_menu_window_get_type(void);

const gchar *brisk_menu_window_get_id(BriskMenuWindow *window);
const gchar *brisk_menu_window_get_display_name(BriskMenuWindow *window);
void brisk_menu_window_update_screen_position(BriskMenuWindow *window);
void brisk_menu_window_update_search(BriskMenuWindow *window);
void brisk_menu_window_invalidate_filter(BriskMenuWindow *self, BriskBackend *backend);
void brisk_menu_window_add_item(BriskMenuWindow *window, BriskItem *item, BriskBackend *backend);
void brisk_menu_window_add_section(BriskMenuWindow *window, BriskSection *section,
                                   BriskBackend *backend);
void brisk_menu_window_reset(BriskMenuWindow *window, BriskBackend *backend);

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
