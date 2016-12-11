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
#include "launcher.h"
#include "menu-private.h"
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

struct _BriskMenuLauncherClass {
        GObjectClass parent_class;
};

/**
 * BriskMenuLauncher is the toplevel window type used within the applet.
 */
struct _BriskMenuLauncher {
        GObject parent;
        GdkAppLaunchContext *context;
        GdkDisplay *display;
};

G_DEFINE_TYPE(BriskMenuLauncher, brisk_menu_launcher, G_TYPE_OBJECT)

static void brisk_menu_launcher_app_launched(BriskMenuLauncher *self, GAppInfo *info,
                                             GVariant *data, GAppLaunchContext *context);
static void brisk_menu_launcher_app_failed(BriskMenuLauncher *self, gchar *startup_id,
                                           GAppLaunchContext *context);

/**
 * brisk_menu_launcher_new:
 *
 * Construct a new BriskMenuLauncher object
 */
BriskMenuLauncher *brisk_menu_launcher_new()
{
        return g_object_new(BRISK_TYPE_MENU_LAUNCHER, NULL);
}

/**
 * brisk_menu_launcher_dispose:
 *
 * Clean up a BriskMenuLauncher instance
 */
static void brisk_menu_launcher_dispose(GObject *obj)
{
        BriskMenuLauncher *self = NULL;

        self = BRISK_MENU_LAUNCHER(obj);
        g_clear_object(&self->context);

        G_OBJECT_CLASS(brisk_menu_launcher_parent_class)->dispose(obj);
}

/**
 * brisk_menu_launcher_class_init:
 *
 * Handle class initialisation
 */
static void brisk_menu_launcher_class_init(BriskMenuLauncherClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_menu_launcher_dispose;
}

/**
 * brisk_menu_launcher_init:
 *
 * Handle construction of the BriskMenuLauncher
 */
static void brisk_menu_launcher_init(BriskMenuLauncher *self)
{
        GdkDisplay *display = NULL;

        display = gdk_display_get_default();
        self->context = gdk_display_get_app_launch_context(display);

        g_signal_connect_swapped(self->context,
                                 "launched",
                                 G_CALLBACK(brisk_menu_launcher_app_launched),
                                 self);
        g_signal_connect_swapped(self->context,
                                 "launch-failed",
                                 G_CALLBACK(brisk_menu_launcher_app_launched),
                                 self);
}

void brisk_menu_launcher_start(BriskMenuLauncher *self, GtkWidget *parent, GAppInfo *app_info)
{
        GdkScreen *screen = NULL;
        GIcon *icon = NULL;

        if (parent) {
                screen = gtk_widget_get_screen(parent);
        } else {
                screen = gdk_screen_get_default();
        }

        icon = g_app_info_get_icon(app_info);

        gdk_app_launch_context_set_screen(self->context, screen);
        if (icon) {
                gdk_app_launch_context_set_icon(self->context, icon);
        }

        self->display = gdk_screen_get_display(screen);

        /* We may support DnD URIs onto the icons at some point, not for now. */
        g_app_info_launch(app_info, NULL, G_APP_LAUNCH_CONTEXT(self->context), NULL);
}

/**
 * Handle the launch of an application so that we can remove the startup notification
 * from the desktop environment.
 */
static void brisk_menu_launcher_app_launched(BriskMenuLauncher *self, GAppInfo *info,
                                             GVariant *data, GAppLaunchContext *context)
{
        GVariantIter iter = { 0 };
        GVariant *elem = NULL;
        gchar *startup_id = NULL;
        GdkDisplay *display = NULL;

        g_variant_iter_init(&iter, data);
        while ((elem = g_variant_iter_next_value(&iter)) != NULL) {
                GVariant *value = NULL;
                gchar *key = NULL;

                g_variant_get(elem, "{sv}", &key, &value);

                if (!g_variant_is_of_type(value, G_VARIANT_TYPE_STRING)) {
                        continue;
                }
                if (g_str_equal(key, "startup-notification-id")) {
                        g_variant_get(value, "s", &startup_id);
                }
                g_free(key);
                g_variant_unref(value);

                if (startup_id) {
                        break;
                }
        }

        if (!startup_id) {
                return;
        }

        g_message("Launched: %s %s", g_app_info_get_name(info), startup_id);
        gdk_display_notify_startup_complete(self->display, startup_id);
        g_free(startup_id);
}

/**
 * Handle the failure of an application to launch, so that we may notify the user
 * about it, and also remove the startup notification from the desktop environment.
 */
static void brisk_menu_launcher_app_failed(BriskMenuLauncher *self, gchar *startup_id,
                                           GAppLaunchContext *context)
{
        g_message("Failed to launch: %s", startup_id);
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
