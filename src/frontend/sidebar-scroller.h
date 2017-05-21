/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2017 Stefan Ric <stfric369@gmail.com>
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

typedef struct _BriskMenuSidebarScroller BriskMenuSidebarScroller;
typedef struct _BriskMenuSidebarScrollerClass BriskMenuSidebarScrollerClass;

#define BRISK_TYPE_MENU_SIDEBAR_SCROLLER brisk_menu_sidebar_scroller_get_type()
#define BRISK_MENU_SIDEBAR_SCROLLER(o)                                                             \
        (G_TYPE_CHECK_INSTANCE_CAST((o),                                                           \
                                    BRISK_TYPE_MENU_SIDEBAR_SCROLLER,                              \
                                    BriskMenuSidebarScroller))
#define BRISK_IS_MENU_SIDEBAR_SCROLLER(o)                                                          \
        (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_MENU_SIDEBAR_SCROLLER))
#define BRISK_MENU_SIDEBAR_SCROLLER_CLASS(o)                                                       \
        (G_TYPE_CHECK_CLASS_CAST((o),                                                              \
                                 BRISK_TYPE_MENU_SIDEBAR_SCROLLER,                                 \
                                 BriskMenuSidebarScrollerClass))
#define BRISK_IS_MENU_SIDEBAR_SCROLLER_CLASS(o)                                                    \
        (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_MENU_SIDEBAR_SCROLLER))
#define BRISK_MENU_SIDEBAR_SCROLLER_GET_CLASS(o)                                                   \
        (G_TYPE_INSTANCE_GET_CLASS((o),                                                            \
                                   BRISK_TYPE_MENU_SIDEBAR_SCROLLER,                               \
                                   BriskMenuSidebarScrollerClass))

/**
 * Construct a new BriskMenuSidebarScroller
 */
GtkWidget *brisk_menu_sidebar_scroller_new(void);

GType brisk_menu_sidebar_scroller_get_type(void);

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
