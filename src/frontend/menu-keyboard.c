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

BRISK_BEGIN_PEDANTIC
#include "menu-private.h"
BRISK_END_PEDANTIC

/**
 * Handle hiding the menu when it comes to the shortcut key only.
 * i.e. the Super_L key.
 */
gboolean brisk_menu_window_key_press(BriskMenuWindow *self, GdkEvent *event,
                                     __brisk_unused__ gpointer v)
{
        autofree(gchar) *accel_name = NULL;
        GdkModifierType lock_masks = GDK_MOD2_MASK | GDK_LOCK_MASK | GDK_MOD5_MASK;
        guint mods;

        if (!self->shortcut) {
                return GDK_EVENT_PROPAGATE;
        }

        /* Unset mask of the lock keys */
        mods = event->key.state & ~(lock_masks);

        accel_name = gtk_accelerator_name(event->key.keyval, mods);
        if (!accel_name || g_ascii_strcasecmp(self->shortcut, accel_name) != 0) {
                return GDK_EVENT_PROPAGATE;
        }

        gtk_widget_hide(GTK_WIDGET(self));
        return GDK_EVENT_STOP;
}

/**
 * Handle the escape key being hit so we can hide again
 */
gboolean brisk_menu_window_key_release(BriskMenuWindow *self, GdkEvent *event,
                                       __brisk_unused__ gpointer v)
{
        if (event->key.keyval == GDK_KEY_Escape) {
                gtk_widget_hide(GTK_WIDGET(self));
                return GDK_EVENT_STOP;
        }
        return GDK_EVENT_PROPAGATE;
}

/**
 * Called in idle once back out of the event
 */
static gboolean toggle_menu(BriskMenuWindow *self)
{
        gboolean vis = !gtk_widget_get_visible(GTK_WIDGET(self));
        if (vis) {
                /* Cheap trick to ensure we reset our active position */
                brisk_menu_window_set_parent_position(self, self->position);
                /* Ensure we're in the appropriate place */
                brisk_menu_window_update_screen_position(self);
        }

        gtk_widget_set_visible(GTK_WIDGET(self), vis);
        return FALSE;
}

/**
 * Handle global hotkey press
 */
static void hotkey_cb(__brisk_unused__ GdkEvent *event, gpointer v)
{
        g_idle_add((GSourceFunc)toggle_menu, v);
}

/**
 * Update the applet hotkey in accordance with settings
 */
void brisk_menu_window_update_hotkey(BriskMenuWindow *self, gchar *key)
{
        if (self->shortcut) {
                brisk_key_binder_unbind(self->binder, self->shortcut);
                g_clear_pointer(&self->shortcut, g_free);
        }

        if (!brisk_key_binder_bind(self->binder, key, hotkey_cb, self)) {
                g_message("Failed to bind keyboard shortcut");
                return;
        }

        self->shortcut = g_strdup(key);
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
