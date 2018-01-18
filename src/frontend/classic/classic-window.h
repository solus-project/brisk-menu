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

typedef struct _BriskClassicWindow BriskClassicWindow;
typedef struct _BriskClassicWindowClass BriskClassicWindowClass;

struct _BriskClassicWindowClass {
        BriskMenuWindowClass parent_class;
};

/**
 * BriskClassicWindow implements support for favourites in Brisk
 */
struct _BriskClassicWindow {
        BriskMenuWindow parent;

        GtkWidget *relative_to;

        /* Categories */
        GtkWidget *sidebar_scroll;

        /* VBox for the sidebar container */
        GtkWidget *sidebar_wrap;

        /* Actual applications */
        GtkWidget *apps;
        GtkWidget *apps_scroll;

        /* CSS Provider */
        GtkCssProvider *css;

        /* Session management buttons */
        GtkWidget *button_lock;
        GtkWidget *button_logout;
        GtkWidget *button_shutdown;
};

#define BRISK_TYPE_CLASSIC_WINDOW brisk_classic_window_get_type()
#define BRISK_CLASSIC_WINDOW(o)                                                                    \
        (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_CLASSIC_WINDOW, BriskClassicWindow))
#define BRISK_IS_CLASSIC_WINDOW(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_CLASSIC_WINDOW))
#define BRISK_CLASSIC_WINDOW_CLASS(o)                                                              \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_CLASSIC_WINDOW, BriskClassicWindowClass))
#define BRISK_IS_CLASSIC_WINDOW_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_CLASSIC_WINDOW))
#define BRISK_CLASSIC_WINDOW_GET_CLASS(o)                                                          \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_CLASSIC_WINDOW, BriskClassicWindowClass))

GType brisk_classic_window_get_type(void);

BriskMenuWindow *brisk_classic_window_new(GtkWidget *relative_to);

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
