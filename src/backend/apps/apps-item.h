/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2017 Ikey Doherty <ikey@solus-project.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <glib-object.h>

#include "../item.h"

G_BEGIN_DECLS

typedef struct _BriskAppsItem BriskAppsItem;
typedef struct _BriskAppsItemClass BriskAppsItemClass;

#define BRISK_TYPE_APPS_ITEM brisk_apps_item_get_type()
#define BRISK_APPS_ITEM(o) (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_APPS_ITEM, BriskAppsItem))
#define BRISK_IS_APPS_ITEM(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_APPS_ITEM))
#define BRISK_APPS_ITEM_CLASS(o)                                                                   \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_APPS_ITEM, BriskAppsItemClass))
#define BRISK_IS_APPS_ITEM_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_APPS_ITEM))
#define BRISK_APPS_ITEM_GET_CLASS(o)                                                               \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_APPS_ITEM, BriskAppsItemClass))

GType brisk_apps_item_get_type(void);

BriskItem *brisk_apps_item_new(GDesktopAppInfo *info);

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
