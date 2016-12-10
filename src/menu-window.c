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
#include "menu-private.h"
#include "menu-window.h"
#include <gtk/gtk.h>
SOLUS_END_PEDANTIC

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
static void sol_menu_window_init(SolMenuWindow *self)
{
        GtkWidget *layout = NULL;
        GtkWidget *widget = NULL;
        GtkWidget *content = NULL;

        /* Create the main layout (Vertical search/content */
        layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_add(GTK_CONTAINER(self), layout);

        /* Create search entry */
        widget = gtk_search_entry_new();
        gtk_box_pack_start(GTK_BOX(layout), widget, FALSE, FALSE, 0);
        gtk_entry_set_placeholder_text(GTK_ENTRY(widget), "Type to search\u2026");
        self->search = widget;

        /* Content layout */
        content = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(layout), content, TRUE, TRUE, 0);

        /* Sidebar for categories */
        widget = gtk_list_box_new();
        gtk_box_pack_start(GTK_BOX(content), widget, FALSE, FALSE, 0);
        self->sidebar = widget;

        /* Stick a vsep in for visual separation */
        widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_box_pack_start(GTK_BOX(content), widget, FALSE, FALSE, 2);

        /* Application launcher display */
        widget = gtk_list_box_new();
        gtk_box_pack_start(GTK_BOX(content), widget, TRUE, TRUE, 0);
        self->apps = widget;

        sol_menu_window_load_menus(self);
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
