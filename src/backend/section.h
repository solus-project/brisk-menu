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

#pragma once

#include "item.h"
#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _BriskSection BriskSection;
typedef struct _BriskSectionClass BriskSectionClass;

struct _BriskSectionClass {
        GInitiallyUnownedClass parent_class;

        /* These must be implemented by subclasses */
        const gchar *(*get_id)(BriskSection *);
        const gchar *(*get_name)(BriskSection *);
        const GIcon *(*get_icon)(BriskSection *);
        const gchar *(*get_backend_id)(BriskSection *);

        gboolean (*can_show_item)(BriskSection *, BriskItem *);

        gint (*get_sort_order)(BriskSection *, BriskItem *);

        gpointer padding[12];
};

/**
 * BriskSection provides the Section concept for sidebar entries within
 * the Brisk Menu
 */
struct _BriskSection {
        GInitiallyUnowned parent;
};

#define BRISK_TYPE_SECTION brisk_section_get_type()
#define BRISK_SECTION(o) (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_SECTION, BriskSection))
#define BRISK_IS_SECTION(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_SECTION))
#define BRISK_SECTION_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_SECTION, BriskSectionClass))
#define BRISK_IS_SECTION_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_SECTION))
#define BRISK_SECTION_GET_CLASS(o)                                                                 \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_SECTION, BriskSectionClass))

GType brisk_section_get_type(void);

/* API Methods Follow */

/* Core vfuncs required by everyone */
const gchar *brisk_section_get_id(BriskSection *section);
const gchar *brisk_section_get_name(BriskSection *section);
const GIcon *brisk_section_get_icon(BriskSection *section);
const gchar *brisk_section_get_backend_id(BriskSection *section);
gboolean brisk_section_can_show_item(BriskSection *section, BriskItem *item);
gint brisk_section_get_sort_order(BriskSection *section, BriskItem *item);

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
