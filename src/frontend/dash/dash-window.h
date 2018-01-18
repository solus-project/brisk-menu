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

#include "../menu-private.h"
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BriskDashWindow BriskDashWindow;
typedef struct _BriskDashWindowClass BriskDashWindowClass;

struct _BriskDashWindowClass {
        BriskMenuWindowClass parent_class;
};

/**
 * BriskDashWindow implements support for favourites in Brisk
 */
struct _BriskDashWindow {
        BriskMenuWindow parent;

        /* Actual applications */
        GtkWidget *apps;
        GtkWidget *apps_scroll;

        GtkWidget *categories_scroll;

        GtkCssProvider *css;
};

#define BRISK_TYPE_DASH_WINDOW brisk_dash_window_get_type()
#define BRISK_DASH_WINDOW(o)                                                                       \
        (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_DASH_WINDOW, BriskDashWindow))
#define BRISK_IS_DASH_WINDOW(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_DASH_WINDOW))
#define BRISK_DASH_WINDOW_CLASS(o)                                                                 \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_DASH_WINDOW, BriskDashWindowClass))
#define BRISK_IS_DASH_WINDOW_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_DASH_WINDOW))
#define BRISK_DASH_WINDOW_GET_CLASS(o)                                                             \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_DASH_WINDOW, BriskDashWindowClass))

GType brisk_dash_window_get_type(void);

BriskMenuWindow *brisk_dash_window_new(GtkWidget *relative_to);

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
