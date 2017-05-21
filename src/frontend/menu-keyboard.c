/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016-2017 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "entry-button.h"
#include "menu-private.h"
BRISK_END_PEDANTIC

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

void brisk_menu_window_key_activate(BriskMenuWindow *self, __brisk_unused__ gpointer v)
{
        autofree(GList) *kids = NULL;
        GList *elem = NULL;
        BriskMenuEntryButton *button = NULL;

        kids = gtk_container_get_children(GTK_CONTAINER(self->apps));

        for (elem = kids; elem; elem = elem->next) {
                GtkWidget *widget = elem->data;

                if (!gtk_widget_get_visible(widget) || !gtk_widget_get_child_visible(widget)) {
                        continue;
                }

                button = BRISK_MENU_ENTRY_BUTTON(gtk_bin_get_child(GTK_BIN(widget)));
                break;
        }
        if (!button) {
                return;
        }
        brisk_menu_entry_button_launch(button);
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
