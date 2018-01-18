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

#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>
#include <matemenu-tree.h>

#include "launcher.h"

G_BEGIN_DECLS

typedef struct _BriskMenuDesktopButton BriskMenuDesktopButton;
typedef struct _BriskMenuDesktopButtonClass BriskMenuDesktopButtonClass;

#define BRISK_TYPE_MENU_DESKTOP_BUTTON brisk_menu_desktop_button_get_type()
#define BRISK_MENU_DESKTOP_BUTTON(o)                                                               \
        (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_MENU_DESKTOP_BUTTON, BriskMenuDesktopButton))
#define BRISK_IS_MENU_DESKTOP_BUTTON(o)                                                            \
        (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_MENU_DESKTOP_BUTTON))
#define BRISK_MENU_DESKTOP_BUTTON_CLASS(o)                                                         \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_MENU_DESKTOP_BUTTON, BriskMenuDesktopButtonClass))
#define BRISK_IS_MENU_DESKTOP_BUTTON_CLASS(o)                                                      \
        (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_MENU_DESKTOP_BUTTON))
#define BRISK_MENU_DESKTOP_BUTTON_GET_CLASS(o)                                                     \
        (G_TYPE_INSTANCE_GET_CLASS((o),                                                            \
                                   BRISK_TYPE_MENU_DESKTOP_BUTTON,                                 \
                                   BriskMenuDesktopButtonClass))

/**
 * Construct a new BriskMenuDesktopButton from the given desktop entry
 */
GtkWidget *brisk_menu_desktop_button_new(BriskMenuLauncher *launcher, GAppInfo *desktop);

GType brisk_menu_desktop_button_get_type(void);

G_END_DECLS

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
