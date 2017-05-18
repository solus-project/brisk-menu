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

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "menu-private.h"
#include <glib/gi18n.h>
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

static void brisk_menu_window_logout_cb(__brisk_unused__ GObject *obj, GAsyncResult *res,
                                        gpointer v)
{
        autofree(GError) *error = NULL;
        BriskMenuWindow *self = v;

        gnome_session_manager_call_logout_finish(self->session, res, &error);
        if (error) {
                g_warning("Error logging out: %s", error->message);
        }
}

static gboolean brisk_menu_window_logout_real(BriskMenuWindow *self)
{
        if (!self->session) {
                return FALSE;
        }

        gnome_session_manager_call_logout(self->session,
                                          0,
                                          NULL,
                                          brisk_menu_window_logout_cb,
                                          self);
        return FALSE;
}

/**
 * Handle log out
 */
static void brisk_menu_window_logout(BriskMenuWindow *self, __brisk_unused__ gpointer v)
{
        gtk_widget_hide(GTK_WIDGET(self));

        g_idle_add((GSourceFunc)brisk_menu_window_logout_real, self);
}

static void brisk_menu_window_shutdown_cb(__brisk_unused__ GObject *obj, GAsyncResult *res,
                                          gpointer v)
{
        autofree(GError) *error = NULL;
        BriskMenuWindow *self = v;

        gnome_session_manager_call_shutdown_finish(self->session, res, &error);
        if (error) {
                g_warning("Error shutting down: %s", error->message);
        }
}

static inline gboolean brisk_menu_window_shutdown_real(BriskMenuWindow *self)
{
        if (!self->session) {
                return FALSE;
        }

        gnome_session_manager_call_shutdown(self->session,
                                            NULL,
                                            brisk_menu_window_shutdown_cb,
                                            self);
        return FALSE;
}

/**
 * Handle shut down
 */
static void brisk_menu_window_shutdown(BriskMenuWindow *self, __brisk_unused__ gpointer v)
{
        gtk_widget_hide(GTK_WIDGET(self));
        g_idle_add((GSourceFunc)brisk_menu_window_shutdown_real, self);
}

static void brisk_menu_window_lock_cb(__brisk_unused__ GObject *obj, GAsyncResult *res, gpointer v)
{
        autofree(GError) *error = NULL;
        BriskMenuWindow *self = v;

        mate_screen_saver_call_lock_finish(self->saver, res, &error);
        if (error) {
                g_warning("Error locking screen: %s", error->message);
        }
}

static inline gboolean brisk_menu_window_lock_real(BriskMenuWindow *self)
{
        if (!self->saver) {
                return FALSE;
        }
        mate_screen_saver_call_lock(self->saver, NULL, brisk_menu_window_lock_cb, self);
        return FALSE;
}

/**
 * Handle lock
 */
static void brisk_menu_window_lock(BriskMenuWindow *self, __brisk_unused__ gpointer v)
{
        gtk_widget_hide(GTK_WIDGET(self));
        g_idle_add((GSourceFunc)brisk_menu_window_lock_real, self);
}

/**
 * Create the graphical buttons for session control
 */
void brisk_menu_window_setup_session_controls(BriskMenuWindow *self)
{
        GtkWidget *widget = NULL;
        GtkWidget *box = NULL;
        GtkStyleContext *style = NULL;

        box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_margin_bottom(box, 4);

        gtk_box_pack_end(GTK_BOX(self->sidebar_wrap), box, FALSE, FALSE, 0);
        gtk_widget_set_halign(box, GTK_ALIGN_CENTER);

        /* Add a separator for visual consistency */
        widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_pack_end(GTK_BOX(self->sidebar_wrap), widget, FALSE, FALSE, 3);

        /* Logout */
        widget = gtk_button_new_from_icon_name("brisk_system-log-out-symbolic", GTK_ICON_SIZE_MENU);
        self->button_logout = widget;
        g_signal_connect_swapped(widget, "clicked", G_CALLBACK(brisk_menu_window_logout), self);
        gtk_widget_set_tooltip_text(widget, _("End the current session"));
        gtk_widget_set_can_focus(widget, FALSE);
        gtk_container_add(GTK_CONTAINER(box), widget);
        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_FLAT);

        /* Lock */
        widget = gtk_button_new_from_icon_name("system-lock-screen-symbolic",
                                               GTK_ICON_SIZE_SMALL_TOOLBAR);
        self->button_lock = widget;
        g_signal_connect_swapped(widget, "clicked", G_CALLBACK(brisk_menu_window_lock), self);
        gtk_widget_set_tooltip_text(widget, _("Lock the screen"));
        gtk_widget_set_can_focus(widget, FALSE);
        gtk_container_add(GTK_CONTAINER(box), widget);
        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_FLAT);

        /* Shutdown */
        widget =
            gtk_button_new_from_icon_name("system-shutdown-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
        self->button_shutdown = widget;
        g_signal_connect_swapped(widget, "clicked", G_CALLBACK(brisk_menu_window_shutdown), self);
        gtk_widget_set_tooltip_text(widget, _("Turn off the device"));
        gtk_widget_set_can_focus(widget, FALSE);
        gtk_container_add(GTK_CONTAINER(box), widget);
        style = gtk_widget_get_style_context(widget);
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_FLAT);
}

gboolean brisk_menu_window_setup_session(BriskMenuWindow *self)
{
        autofree(GError) *error = NULL;
        gboolean can_shutdown = FALSE;
        __brisk_unused__ gboolean is_active = FALSE;

        return FALSE;

        /* Sort out gnome-session dbus */
        self->session = gnome_session_manager_proxy_new_for_bus_sync(
            G_BUS_TYPE_SESSION,
            G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START | G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
            "org.gnome.SessionManager",
            "/org/gnome/SessionManager",
            NULL,
            &error);
        if (error) {
                g_warning("Failed to contact org.gnome.SessionManager: %s\n", error->message);
                g_error_free(error);

                gtk_widget_set_sensitive(self->button_shutdown, FALSE);
                gtk_widget_set_sensitive(self->button_logout, FALSE);
                goto saver_init;
        }

        /* Set sensitive according to policy */
        gnome_session_manager_call_can_shutdown_sync(self->session, &can_shutdown, NULL, NULL);
        gtk_widget_set_sensitive(self->button_shutdown, can_shutdown);

saver_init:
        self->saver =
            mate_screen_saver_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                                     G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                     "org.mate.ScreenSaver",
                                                     "/org/mate/ScreenSaver",
                                                     NULL,
                                                     &error);
        if (error) {
                g_warning("Failed to contact org.mate.ScreenSaver: %s\n", error->message);
                gtk_widget_set_sensitive(self->button_lock, FALSE);
                return FALSE;
        }

        /* Check the screensaver is *really* running */
        mate_screen_saver_call_get_active_sync(self->saver, &is_active, NULL, &error);
        if (error) {
                gtk_widget_set_sensitive(self->button_lock, FALSE);
                g_warning("org.mate.ScreenSaver not running: %s\n", error->message);
        }
        return FALSE;
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
