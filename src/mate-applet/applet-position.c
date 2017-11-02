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

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "applet.h"
#include "lib/styles.h"
BRISK_END_PEDANTIC

/**
 * brisk_menu_applet_adapt_layout:
 *
 * Update our layout in response to an orientation change.
 * Primarily we're hiding our label automatically here and maximizing the space
 * available to the icon.
 */
void brisk_menu_applet_adapt_layout(BriskMenuApplet *self)
{
        GtkStyleContext *style = NULL;

        style = gtk_widget_get_style_context(self->toggle);

        switch (self->orient) {
        case MATE_PANEL_APPLET_ORIENT_LEFT:
        case MATE_PANEL_APPLET_ORIENT_RIGHT:
                /* Handle vertical panel layout */
                gtk_widget_hide(self->label);
                gtk_widget_set_halign(self->image, GTK_ALIGN_CENTER);
                gtk_style_context_add_class(style, BRISK_STYLE_BUTTON_VERTICAL);
                gtk_widget_set_margin_end(self->image, 0);
                break;
        default:
                /* We're a horizontal panel */
                gtk_widget_set_visible(self->label,
                                       g_settings_get_boolean(self->settings, "label-visible"));
                gtk_widget_set_halign(self->image, GTK_ALIGN_START);
                gtk_style_context_remove_class(style, BRISK_STYLE_BUTTON_VERTICAL);
                gtk_widget_set_margin_end(self->image, 4);
                break;
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
