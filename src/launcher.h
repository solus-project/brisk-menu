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

#pragma once

#include <glib-object.h>

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
