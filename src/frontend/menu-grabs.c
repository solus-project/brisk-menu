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

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "menu-private.h"
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

static gboolean brisk_menu_window_map(GtkWidget *widget, gpointer udata);
static gboolean brisk_menu_window_unmap(GtkWidget *widget, gpointer udata);
static void brisk_menu_window_grab_notify(GtkWidget *widget, gboolean was_grabbed, gpointer udata);
static gboolean brisk_menu_window_button_press(GtkWidget *widget, GdkEvent *event, gpointer udata);
static gboolean brisk_menu_window_grab_broken(GtkWidget *widget, GdkEvent *event, gpointer udata);
static void brisk_menu_window_grab(BriskMenuWindow *self);
static void brisk_menu_window_ungrab(BriskMenuWindow *self);

/**
 * Borrowed from gdkseatdefault.c
 */
#define KEYBOARD_EVENTS (GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK | GDK_FOCUS_CHANGE_MASK)
#define POINTER_EVENTS                                                                             \
        (GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |               \
         GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK | GDK_ENTER_NOTIFY_MASK |                        \
         GDK_LEAVE_NOTIFY_MASK | GDK_PROXIMITY_IN_MASK | GDK_PROXIMITY_OUT_MASK)

/**
 * Set up grabbing support within the menu
 */
void brisk_menu_window_configure_grabs(BriskMenuWindow *self)
{
        g_signal_connect(self, "map-event", G_CALLBACK(brisk_menu_window_map), NULL);
        g_signal_connect(self, "unmap-event", G_CALLBACK(brisk_menu_window_unmap), NULL);
        g_signal_connect(self, "grab-notify", G_CALLBACK(brisk_menu_window_grab_notify), NULL);
        g_signal_connect(self,
                         "button-press-event",
                         G_CALLBACK(brisk_menu_window_button_press),
                         NULL);
        g_signal_connect(self,
                         "grab-broken-event",
                         G_CALLBACK(brisk_menu_window_grab_broken),
                         NULL);
}

/**
 * Mapped on screen, attempt a grab
 */
static gboolean brisk_menu_window_map(GtkWidget *widget, __brisk_unused__ gpointer udata)
{
        GdkWindow *window = NULL;
        BriskMenuWindow *self = NULL;

        self = BRISK_MENU_WINDOW(widget);

        /* Forcibly request focus */
        window = gtk_widget_get_window(widget);
        gdk_window_set_accept_focus(window, TRUE);
        gdk_window_focus(window, GDK_CURRENT_TIME);
        gtk_window_present(GTK_WINDOW(widget));
        gtk_widget_grab_focus(self->search);

        brisk_menu_window_grab(self);

        return GDK_EVENT_STOP;
}

/**
 * We've been made non-visible, so drop our grab if we have it
 */
static gboolean brisk_menu_window_unmap(GtkWidget *widget, __brisk_unused__ gpointer udata)
{
        brisk_menu_window_ungrab(BRISK_MENU_WINDOW(widget));
        return GDK_EVENT_STOP;
}

/**
 * Grab the input events using the GdkSeat
 */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION < 20
/* Pre 3.20 grab behaviour */
static void brisk_menu_window_grab(BriskMenuWindow *self)
{
        GdkDisplay *display = NULL;
        GdkWindow *window = NULL;
        GdkGrabStatus st;
        GdkDeviceManager *manager = NULL;
        GdkDevice *pointer, *keyboard = NULL;

        if (self->grabbed) {
                return;
        }

        window = gtk_widget_get_window(GTK_WIDGET(self));
        if (!window) {
                g_warning("Attempting to grab BriskMenuWindow when not realized");
                return;
        }

        display = gtk_widget_get_display(GTK_WIDGET(self));
        manager = gdk_display_get_device_manager(display);

        pointer = gdk_device_manager_get_client_pointer(manager);
        keyboard = gdk_device_get_associated_device(pointer);

        st = gdk_device_grab(pointer,
                             window,
                             GDK_OWNERSHIP_NONE,
                             TRUE,
                             POINTER_EVENTS,
                             NULL,
                             GDK_CURRENT_TIME);

        if (st != GDK_GRAB_SUCCESS) {
                return;
        }
        if (keyboard) {
                st = gdk_device_grab(keyboard,
                                     window,
                                     GDK_OWNERSHIP_NONE,
                                     TRUE,
                                     KEYBOARD_EVENTS,
                                     NULL,
                                     GDK_CURRENT_TIME);
                if (st != GDK_GRAB_SUCCESS) {
                        gdk_device_ungrab(keyboard, GDK_CURRENT_TIME);
                        return;
                }
        }

        self->grabbed = TRUE;
        gtk_grab_add(GTK_WIDGET(self));
}
#else
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static void brisk_menu_window_grab(BriskMenuWindow *self)
{
        GdkDisplay *display = NULL;
        GdkSeat *seat = NULL;
        GdkWindow *window = NULL;
        GdkSeatCapabilities caps = 0;
        GdkGrabStatus st;

        if (self->grabbed) {
                return;
        }

        window = gtk_widget_get_window(GTK_WIDGET(self));
        if (!window) {
                g_warning("Attempting to grab BriskMenuWindow when not realized");
                return;
        }

        display = gtk_widget_get_display(GTK_WIDGET(self));
        seat = gdk_display_get_default_seat(display);

        if (gdk_seat_get_pointer(seat) != NULL) {
                caps |= GDK_SEAT_CAPABILITY_ALL_POINTING;
        }
        if (gdk_seat_get_keyboard(seat) != NULL) {
                caps |= GDK_SEAT_CAPABILITY_KEYBOARD;
        }

        st = gdk_seat_grab(seat, window, caps, TRUE, NULL, NULL, NULL, NULL);
        if (st == GDK_GRAB_SUCCESS) {
                self->grabbed = TRUE;
                gtk_grab_add(GTK_WIDGET(self));
        }
}
G_GNUC_END_IGNORE_DEPRECATIONS
#endif

/**
 * Ungrab a previous grab by this widget
 */
#if GTK_MAJOR_VERSION == 3 && GTK_MINOR_VERSION < 20
/* Pre 3.20 behaviour */
static void brisk_menu_window_ungrab(BriskMenuWindow *self)
{
        GdkDisplay *display = NULL;
        GdkDeviceManager *manager = NULL;
        GdkDevice *pointer, *keyboard = NULL;

        if (!self->grabbed) {
                return;
        }

        display = gtk_widget_get_display(GTK_WIDGET(self));
        manager = gdk_display_get_device_manager(display);

        pointer = gdk_device_manager_get_client_pointer(manager);
        keyboard = gdk_device_get_associated_device(pointer);

        gtk_grab_remove(GTK_WIDGET(self));

        gdk_device_ungrab(pointer, GDK_CURRENT_TIME);
        if (keyboard) {
                gdk_device_ungrab(keyboard, GDK_CURRENT_TIME);
        }
        self->grabbed = FALSE;
}
#else
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static void brisk_menu_window_ungrab(BriskMenuWindow *self)
{
        GdkDisplay *display = NULL;
        GdkSeat *seat = NULL;

        if (!self->grabbed) {
                return;
        }

        display = gtk_widget_get_display(GTK_WIDGET(self));
        seat = gdk_display_get_default_seat(display);

        gtk_grab_remove(GTK_WIDGET(self));
        gdk_seat_ungrab(seat);
        self->grabbed = FALSE;
}
G_GNUC_END_IGNORE_DEPRECATIONS
#endif

/**
 * Grab was broken, most likely due to a window within our application
 */
static gboolean brisk_menu_window_grab_broken(GtkWidget *widget, __brisk_unused__ GdkEvent *event,
                                              __brisk_unused__ gpointer udata)
{
        BriskMenuWindow *self = NULL;

        self = BRISK_MENU_WINDOW(widget);
        self->grabbed = FALSE;
        return GDK_EVENT_PROPAGATE;
}

/**
 * Grab changed _within_ the application
 *
 * If our grab was broken, i.e. due to some popup menu, and we're still visible,
 * we'll now try and grab focus once more.
 */
static void brisk_menu_window_grab_notify(GtkWidget *widget, gboolean was_grabbed,
                                          __brisk_unused__ gpointer udata)
{
        BriskMenuWindow *self = NULL;

        /* Only interested in unshadowed */
        if (!was_grabbed) {
                return;
        }

        /* And being visible. ofc. */
        if (!gtk_widget_get_visible(widget)) {
                return;
        }

        self = BRISK_MENU_WINDOW(widget);
        brisk_menu_window_grab(self);
}

/**
 * Check for clicks outside the window itself
 */
static gboolean brisk_menu_window_button_press(GtkWidget *widget, GdkEvent *event,
                                               __brisk_unused__ gpointer udata)
{
        gint wx, wy = 0;
        gint ww, wh = 0;

        gtk_window_get_size(GTK_WINDOW(widget), &ww, &wh);
        gtk_window_get_position(GTK_WINDOW(widget), &wx, &wy);

        if ((event->button.x_root < wx || event->button.x_root > (wx + ww)) ||
            (event->button.y_root < wy || event->button.y_root > (wy + wh))) {
                gtk_widget_hide(widget);
                return GDK_EVENT_STOP;
        }
        return GDK_EVENT_PROPAGATE;
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
