/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2017 Brisk Menu Developers
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
#include "menu-window.h"
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

/**
 * brisk_menu_window_show_context:
 *
 * Menu button has requested a context menu be shown for the given item
 */
void brisk_menu_window_show_context(BriskMenuWindow *self, BriskItem *item,
                                    BriskMenuEntryButton *button)
{
        /* TODO: Show menu */
}

/**
* brisk_menu_window_configure_context:
*
* Set up the basics for handling context menus
*/
void brisk_menu_window_configure_context(BriskMenuWindow *self)
{
        /* TODO: Init bits */
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
