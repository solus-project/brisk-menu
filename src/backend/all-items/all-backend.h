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

#include "../backend.h"
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _BriskAllItemsBackend BriskAllItemsBackend;
typedef struct _BriskAllItemsBackendClass BriskAllItemsBackendClass;

#define BRISK_TYPE_ALL_ITEMS_BACKEND brisk_all_items_backend_get_type()
#define BRISK_ALL_ITEMS_BACKEND(o)                                                                 \
        (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_ALL_ITEMS_BACKEND, BriskAllItemsBackend))
#define BRISK_IS_ALL_ITEMS_BACKEND(o)                                                              \
        (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_ALL_ITEMS_BACKEND))
#define BRISK_ALL_ITEMS_BACKEND_CLASS(o)                                                           \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_ALL_ITEMS_BACKEND, BriskAllItemsBackendClass))
#define BRISK_IS_ALL_ITEMS_BACKEND_CLASS(o)                                                        \
        (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_ALL_ITEMS_BACKEND))
#define BRISK_ALL_ITEMS_BACKEND_GET_CLASS(o)                                                       \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_ALL_ITEMS_BACKEND, BriskAllItemsBackendClass))

GType brisk_all_items_backend_get_type(void);

BriskBackend *brisk_all_items_backend_new(void);

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
