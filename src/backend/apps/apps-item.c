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
#include "apps-item.h"
BRISK_END_PEDANTIC

enum { PROP_INFO = 1, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

struct _BriskAppsItemClass {
        BriskItemClass parent_class;
};

/**
 * BriskAppsItem is a type of item for the Brisk menu which is backed by
 * a .desktop file
 */
struct _BriskAppsItem {
        BriskItem parent;
        GDesktopAppInfo *info;
};

G_DEFINE_TYPE(BriskAppsItem, brisk_apps_item, BRISK_TYPE_ITEM)

/**
 * Basic subclassing
 */
static const gchar *brisk_apps_item_get_id(BriskItem *item);
static const gchar *brisk_apps_item_get_name(BriskItem *item);
static const gchar *brisk_apps_item_get_summary(BriskItem *item);
static const GIcon *brisk_apps_item_get_icon(BriskItem *item);
static gboolean brisk_apps_item_matches_search(BriskItem *item, gchar *term);
static gboolean brisk_apps_item_launch(BriskItem *item);

static void brisk_apps_item_set_property(GObject *object, guint id, const GValue *value,
                                         GParamSpec *spec)
{
        BriskAppsItem *self = BRISK_APPS_ITEM(object);

        switch (id) {
        case PROP_INFO:
                self->info = g_value_get_pointer(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void brisk_apps_item_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec)
{
        BriskAppsItem *self = BRISK_APPS_ITEM(object);

        switch (id) {
        case PROP_INFO:
                g_value_set_pointer(value, self->info);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * brisk_apps_item_dispose:
 *
 * Clean up a BriskAppsItem instance
 */
static void brisk_apps_item_dispose(GObject *obj)
{
        BriskAppsItem *self = BRISK_APPS_ITEM(obj);

        g_clear_object(&self->info);

        G_OBJECT_CLASS(brisk_apps_item_parent_class)->dispose(obj);
}

/**
 * brisk_apps_item_class_init:
 *
 * Handle class initialisation
 */
static void brisk_apps_item_class_init(BriskAppsItemClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        BriskItemClass *i_class = BRISK_ITEM_CLASS(klazz);

        /* item vtable hookup */
        i_class->get_id = brisk_apps_item_get_id;
        i_class->get_name = brisk_apps_item_get_name;
        i_class->get_summary = brisk_apps_item_get_summary;
        i_class->get_icon = brisk_apps_item_get_icon;
        i_class->matches_search = brisk_apps_item_matches_search;
        i_class->launch = brisk_apps_item_launch;

        /* gobject vtable hookup */
        obj_class->dispose = brisk_apps_item_dispose;
        obj_class->set_property = brisk_apps_item_set_property;
        obj_class->get_property = brisk_apps_item_get_property;

        obj_properties[PROP_INFO] = g_param_spec_pointer("info",
                                                         "The GDesktopAppInfo",
                                                         "Corresponding .desktop file",
                                                         G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

/**
 * brisk_apps_item_init:
 *
 * Handle construction of the BriskAppsItem
 */
static void brisk_apps_item_init(__brisk_unused__ BriskAppsItem *self)
{
}

static const gchar *brisk_apps_item_get_id(BriskItem *item)
{
        BriskAppsItem *self = BRISK_APPS_ITEM(item);
        return g_app_info_get_id(G_APP_INFO(self->info));
}

static const gchar *brisk_apps_item_get_name(BriskItem *item)
{
        BriskAppsItem *self = BRISK_APPS_ITEM(item);
        /* TODO: Consider using display name */
        return g_app_info_get_name(G_APP_INFO(self->info));
}

static const gchar *brisk_apps_item_get_summary(BriskItem *item)
{
        BriskAppsItem *self = BRISK_APPS_ITEM(item);
        return g_app_info_get_description(G_APP_INFO(self->info));
}

static const GIcon *brisk_apps_item_get_icon(BriskItem *item)
{
        BriskAppsItem *self = BRISK_APPS_ITEM(item);
        return g_app_info_get_icon(G_APP_INFO(self->info));
}

/**
 * Not yet implemented
 */
static gboolean brisk_apps_item_matches_search(__brisk_unused__ BriskItem *item,
                                               __brisk_unused__ gchar *term)
{
        return FALSE;
}

/**
 * Not yet implemented
 */
static gboolean brisk_apps_item_launch(__brisk_unused__ BriskItem *item)
{
        return FALSE;
}

/**
 * brisk_apps_item_new:
 *
 * Return a new BriskAppsItem for the given desktop file
 */
BriskItem *brisk_apps_item_new(GDesktopAppInfo *info)
{
        return g_object_new(BRISK_TYPE_APPS_ITEM, "info", info, NULL);
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
