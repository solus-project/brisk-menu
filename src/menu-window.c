/*
 * This file is part of mate-solmenu.
 *
 * Copyright © 2016 Ikey Doherty <ikey@solus-proejct.com>
 *
 * mate-solmenu is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

SOLUS_BEGIN_PEDANTIC
#include "menu-window.h"
#include <gtk/gtk.h>
SOLUS_END_PEDANTIC

struct _SolMenuWindowClass {
        GtkWindowClass parent_class;
};

struct _SolMenuWindow {
        GtkWindow parent;
};

G_DEFINE_TYPE(SolMenuWindow, sol_menu_window, GTK_TYPE_WINDOW)

/**
 * sol_menu_window_new:
 *
 * Construct a new SolMenuWindow object
 */
GtkWidget *sol_menu_window_new()
{
        return g_object_new(SOL_TYPE_MENU_WINDOW, NULL);
}

/**
 * sol_menu_window_dispose:
 *
 * Clean up a SolMenuWindow instance
 */
static void sol_menu_window_dispose(GObject *obj)
{
        G_OBJECT_CLASS(sol_menu_window_parent_class)->dispose(obj);
}

/**
 * sol_menu_window_class_init:
 *
 * Handle class initialisation
 */
static void sol_menu_window_class_init(SolMenuWindowClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = sol_menu_window_dispose;
}

/**
 * sol_menu_window_init:
 *
 * Handle construction of the SolMenuWindow
 */
static void sol_menu_window_init(__solus_unused__ SolMenuWindow *self)
{
        /* TODO: Anything, really. */
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
