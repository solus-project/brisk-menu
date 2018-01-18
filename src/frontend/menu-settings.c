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

#include "util.h"

#include <stdlib.h>

BRISK_BEGIN_PEDANTIC
#include "menu-private.h"
BRISK_END_PEDANTIC

static void brisk_menu_window_settings_changed(GSettings *settings, const gchar *key, gpointer v);

void brisk_menu_window_init_settings(BriskMenuWindow *self)
{
        GtkSettings *gtk_settings = NULL;

        self->settings = g_settings_new("com.solus-project.brisk-menu");
        self->position = GTK_POS_TOP;

        gtk_settings = gtk_settings_get_default();

        /* Make dark-theme key work */
        g_settings_bind(self->settings,
                        "dark-theme",
                        gtk_settings,
                        "gtk-application-prefer-dark-theme",
                        G_SETTINGS_BIND_DEFAULT);

        g_signal_connect(self->settings,
                         "changed",
                         G_CALLBACK(brisk_menu_window_settings_changed),
                         self);
}

void brisk_menu_window_pump_settings(BriskMenuWindow *self)
{
        brisk_menu_window_settings_changed(self->settings, "search-position", self);
        brisk_menu_window_settings_changed(self->settings, "rollover-activate", self);
        brisk_menu_window_settings_changed(self->settings, "hot-key", self);
}

static void brisk_menu_window_settings_changed(GSettings *settings, const gchar *key, gpointer v)
{
        BriskMenuWindow *self = v;
        autofree(gchar) *value = NULL;

        if (g_str_equal(key, "search-position")) {
                self->search_position = g_settings_get_enum(settings, key);
                brisk_menu_window_update_search(self);
                return;
        } else if (g_str_equal(key, "rollover-activate")) {
                self->rollover = g_settings_get_boolean(settings, key);
        } else if (g_str_equal(key, "hot-key")) {
                value = g_settings_get_string(settings, key);
                brisk_menu_window_update_hotkey(self, value);
        }
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
