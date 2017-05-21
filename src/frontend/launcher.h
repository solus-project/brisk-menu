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

#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

#include "backend/item.h"

G_BEGIN_DECLS

typedef struct _BriskMenuLauncher BriskMenuLauncher;
typedef struct _BriskMenuLauncherClass BriskMenuLauncherClass;

#define BRISK_TYPE_MENU_LAUNCHER brisk_menu_launcher_get_type()
#define BRISK_MENU_LAUNCHER(o)                                                                     \
        (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_MENU_LAUNCHER, BriskMenuLauncher))
#define BRISK_IS_MENU_LAUNCHER(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_MENU_LAUNCHER))
#define BRISK_MENU_LAUNCHER_CLASS(o)                                                               \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_MENU_LAUNCHER, BriskMenuLauncherClass))
#define BRISK_IS_MENU_LAUNCHER_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_MENU_LAUNCHER))
#define BRISK_MENU_LAUNCHER_GET_CLASS(o)                                                           \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_MENU_LAUNCHER, BriskMenuLauncherClass))

/**
 * Construct a new BriskMenuLauncher to manage launching of apps
 */
BriskMenuLauncher *brisk_menu_launcher_new(void);

GType brisk_menu_launcher_get_type(void);

/**
 * Start up the given app_info with no URIs and handle startup notification
 * for it.
 */
void brisk_menu_launcher_start(BriskMenuLauncher *self, GtkWidget *parent, GAppInfo *app_info);

/**
 * Start a given Brisk item in the same context as _start, but ask the item to
 * launch itself after our context is prepared.
 */
void brisk_menu_launcher_start_item(BriskMenuLauncher *self, GtkWidget *parent, BriskItem *item);

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
