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
#include "applet.h"
#include "menu-window.h"
#include <gtk/gtk.h>
#include <mate-panel-applet.h>
BRISK_END_PEDANTIC

struct _BriskMenuAppletClass {
        MatePanelAppletClass parent_class;
};

struct _BriskMenuApplet {
        MatePanelApplet parent;
        GtkWidget *toggle;
        GtkWidget *menu;
};

G_DEFINE_TYPE(BriskMenuApplet, brisk_menu_applet, PANEL_TYPE_APPLET)

/**
 * Handle showing of the menu
 */
static gboolean button_clicked_cb(BriskMenuApplet *self, gpointer udata);

/**
 * brisk_menu_applet_dispose:
 *
 * Clean up a BriskMenuApplet instance
 */
static void brisk_menu_applet_dispose(GObject *obj)
{
        BriskMenuApplet *self = NULL;

        self = BRISK_MENU_APPLET(obj);

        /* Tear down the menu */
        if (self->menu) {
                gtk_widget_hide(self->menu);
                g_clear_pointer(&self->menu, gtk_widget_destroy);
        }

        G_OBJECT_CLASS(brisk_menu_applet_parent_class)->dispose(obj);
}

/**
 * brisk_menu_applet_class_init:
 *
 * Handle class initialisation
 */
static void brisk_menu_applet_class_init(BriskMenuAppletClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_menu_applet_dispose;
}

/**
 * brisk_menu_applet_init:
 *
 * Handle construction of the BriskMenuApplet
 */
static void brisk_menu_applet_init(BriskMenuApplet *self)
{
        GtkWidget *toggle, *menu = NULL;

        /* DEMO CODE */
        toggle = gtk_button_new_with_label("Menu");
        gtk_container_add(GTK_CONTAINER(self), toggle);
        g_signal_connect_swapped(toggle, "clicked", G_CALLBACK(button_clicked_cb), self);
        gtk_widget_show_all(toggle);
        self->toggle = toggle;

        /* Construct our menu */
        menu = brisk_menu_window_new();
        self->menu = menu;
}

static gboolean button_clicked_cb(BriskMenuApplet *self, __brisk_unused__ gpointer udata)
{
        gboolean vis = gtk_widget_get_visible(self->menu);

        gtk_widget_set_visible(self->menu, !vis);

        return GDK_EVENT_STOP;
}

static gboolean brisk_menu_applet_factory(MatePanelApplet *applet, const gchar *id, gpointer udata)
{
        if (!g_str_has_prefix(id, "BriskMenu")) {
                return FALSE;
        }
        g_set_application_name("Brisk Menu Launcher");
        /* TODO: Fix things up to be more useful. */
        gtk_widget_show(GTK_WIDGET(applet));
        return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("BriskMenuFactory", BRISK_TYPE_MENU_APPLET, "BriskMenu",
                                      brisk_menu_applet_factory, NULL)

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
