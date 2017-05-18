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

#include <gio/gio.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _BriskItem BriskItem;
typedef struct _BriskItemClass BriskItemClass;

struct _BriskItemClass {
        GObjectClass parent_class;

        /* These must be implemented by subclasses */
        const gchar *(*get_id)(BriskItem *);
        const gchar *(*get_name)(BriskItem *);
        const gchar *(*get_summary)(BriskItem *);
        const GIcon *(*get_icon)(BriskItem *);
        const gchar *(*get_backend_id)(BriskItem *);

        /* If the subclass supports searching, override this */
        gboolean (*matches_search)(BriskItem *, gchar *);

        /* Support launching through primary click action */
        gboolean (*launch)(BriskItem *);

        gpointer padding[12];
};

/**
 * BriskItem represents a single "thing" within Brisk, such as an application
 * launcher or file.
 */
struct _BriskItem {
        GObject parent;
};

#define BRISK_TYPE_ITEM brisk_item_get_type()
#define BRISK_ITEM(o) (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_ITEM, BriskItem))
#define BRISK_IS_ITEM(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_ITEM))
#define BRISK_ITEM_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_ITEM, BriskItemClass))
#define BRISK_IS_ITEM_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_ITEM))
#define BRISK_ITEM_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_ITEM, BriskItemClass))

GType brisk_item_get_type(void);

/* API Methods Follow */

/* Core vfuncs required by everyone */
const gchar *brisk_item_get_id(BriskItem *item);
const gchar *brisk_item_get_name(BriskItem *item);
const gchar *brisk_item_get_summary(BriskItem *item);
const GIcon *brisk_item_get_icon(BriskItem *item);
const gchar *brisk_item_get_backend_id(BriskItem *item);
gboolean brisk_item_matches_search(BriskItem *item, gchar *term);

/* Attempt to load for the first time */
gboolean brisk_item_launch(BriskItem *backend);

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
