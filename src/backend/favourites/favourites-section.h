/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2017-2018 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include <gio/gio.h>
#include <glib-object.h>

#include "../section.h"
#include "favourites-backend.h"

G_BEGIN_DECLS

typedef struct _BriskFavouritesSection BriskFavouritesSection;
typedef struct _BriskFavouritesSectionClass BriskFavouritesSectionClass;

#define BRISK_TYPE_FAVOURITES_SECTION brisk_favourites_section_get_type()
#define BRISK_FAVOURITES_SECTION(o)                                                                \
        (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_FAVOURITES_SECTION, BriskFavouritesSection))
#define BRISK_IS_FAVOURITES_SECTION(o)                                                             \
        (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_FAVOURITES_SECTION))
#define BRISK_FAVOURITES_SECTION_CLASS(o)                                                          \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_FAVOURITES_SECTION, BriskFavouritesSectionClass))
#define BRISK_IS_FAVOURITES_SECTION_CLASS(o)                                                       \
        (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_FAVOURITES_SECTION))
#define BRISK_FAVOURITES_SECTION_GET_CLASS(o)                                                      \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_FAVOURITES_SECTION, BriskFavouritesSectionClass))

GType brisk_favourites_section_get_type(void);

BriskSection *brisk_favourites_section_new(BriskFavouritesBackend *backend);

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
