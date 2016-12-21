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

#include "util.h"

BRISK_BEGIN_PEDANTIC
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
