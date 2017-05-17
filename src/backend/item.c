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

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "item.h"
BRISK_END_PEDANTIC

G_DEFINE_TYPE(BriskItem, brisk_item, G_TYPE_OBJECT)

/**
 * brisk_item_dispose:
 *
 * Clean up a BriskItem instance
 */
static void brisk_item_dispose(GObject *obj)
{
        G_OBJECT_CLASS(brisk_item_parent_class)->dispose(obj);
}

/**
 * brisk_item_class_init:
 *
 * Handle class initialisation
 */
static void brisk_item_class_init(BriskItemClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_item_dispose;
}

/**
 * brisk_item_init:
 *
 * Handle construction of the BriskItem
 */
static void brisk_item_init(__brisk_unused__ BriskItem *self)
{
}

/**
 * brisk_item_get_id:
 *
 * Returns the unique ID for this item within the backend
 * @note This string belongs to the backend, and must not be freed by the caller
 */
const gchar *brisk_item_get_id(BriskItem *item)
{
        g_assert(item != NULL);
        BriskItemClass *klazz = BRISK_ITEM_GET_CLASS(item);
        g_assert(klazz->get_id != NULL);
        return klazz->get_id(item);
}

/**
 * brisk_item_get_name:
 *
 * Returns the item name used when display the item in the menu
 * @note This string belongs to the item, and must not be freed by the caller
 */
const gchar *brisk_item_get_name(BriskItem *item)
{
        g_assert(item != NULL);
        BriskItemClass *klazz = BRISK_ITEM_GET_CLASS(item);
        g_assert(klazz->get_name != NULL);
        return klazz->get_name(item);
}

/**
 * brisk_item_get_summary:
 *
 * Returns the summary used when display the item in the menu
 * @note This string belongs to the item, and must not be freed by the caller
 */
const gchar *brisk_item_get_summary(BriskItem *item)
{
        g_assert(item != NULL);
        BriskItemClass *klazz = BRISK_ITEM_GET_CLASS(item);
        g_assert(klazz->get_summary != NULL);
        return klazz->get_summary(item);
}

/**
 * brisk_item_get_icon:
 *
 * Returns the icon used to display this item in the menu
 */
const GIcon *brisk_item_get_icon(BriskItem *item)
{
        g_assert(item != NULL);
        BriskItemClass *klazz = BRISK_ITEM_GET_CLASS(item);
        if (!klazz->get_icon) {
                return NULL;
        }
        return klazz->get_icon(item);
}

/**
 * brisk_item_matches_search:
 *
 * Returns true if the item matches the given search term
 */
gboolean brisk_item_matches_search(BriskItem *item, gchar *term)
{
        g_assert(item != NULL);
        BriskItemClass *klazz = BRISK_ITEM_GET_CLASS(item);
        g_return_val_if_fail(klazz->matches_search != NULL, FALSE);
        return klazz->matches_search(item, term);
}

/**
 * brisk_item_launch:
 *
 * Attempt to launch the item
 */
gboolean brisk_item_launch(BriskItem *item)
{
        g_assert(item != NULL);
        BriskItemClass *klazz = BRISK_ITEM_GET_CLASS(item);
        g_return_val_if_fail(klazz->launch != NULL, FALSE);
        return klazz->launch(item);
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
