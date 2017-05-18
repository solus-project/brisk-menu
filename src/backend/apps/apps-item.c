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
#include <string.h>

BRISK_BEGIN_PEDANTIC
#include "apps-item.h"
BRISK_END_PEDANTIC

enum { PROP_INFO = 1, N_PROPS };

DEF_AUTOFREE(gchar, g_free)

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
static const char *brisk_apps_item_get_backend_id(BriskItem *item);
static gboolean brisk_apps_item_matches_search(BriskItem *item, gchar *term);
static gboolean brisk_apps_item_launch(BriskItem *item);

static void brisk_apps_item_set_property(GObject *object, guint id, const GValue *value,
                                         GParamSpec *spec)
{
        BriskAppsItem *self = BRISK_APPS_ITEM(object);

        switch (id) {
        case PROP_INFO:
                self->info = g_value_dup_object(value);
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
        i_class->get_backend_id = brisk_apps_item_get_backend_id;
        i_class->matches_search = brisk_apps_item_matches_search;
        i_class->launch = brisk_apps_item_launch;

        /* gobject vtable hookup */
        obj_class->dispose = brisk_apps_item_dispose;
        obj_class->set_property = brisk_apps_item_set_property;
        obj_class->get_property = brisk_apps_item_get_property;

        obj_properties[PROP_INFO] = g_param_spec_object("info",
                                                        "The GDesktopAppInfo",
                                                        "Corresponding .desktop file",
                                                        G_TYPE_DESKTOP_APP_INFO,
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

static const char *brisk_apps_item_get_backend_id(__brisk_unused__ BriskItem *item)
{
        return "apps";
}

__brisk_pure__ static gboolean brisk_apps_array_contains(const gchar **fields, size_t n_fields,
                                                         const gchar *term)
{
        for (size_t i = 0; i < n_fields; i++) {
                if (!fields[i]) {
                        continue;
                }
                autofree(gchar) *contents = g_strstrip(g_ascii_strdown(fields[i], -1));
                if (g_str_match_string(term, contents, TRUE)) {
                        return TRUE;
                }
                if (strstr(contents, term)) {
                        return TRUE;
                }
        }
        return FALSE;
}

/**
 * brisk_apps_item_matches_search:
 *
 * This function will handle filtering the selection based on the active search
 * term. It looks for the string within a number of the entry's fields, and will
 * hide them if they don't turn up.
 *
 * Note: This function uses g_str_match_string so that ASCII alternatives are
 * searched. This allows searching for text containing accents, etc, so that
 * the menu can be more useful in more locales.
 *
 * This could probably be improved in future to generate internal state to allow
 * the search itself to be sorted based on the results, with the "most similar"
 * appearing near the top.
 */
__brisk_pure__ static gboolean brisk_apps_item_matches_search(BriskItem *item, gchar *term)
{
        BriskAppsItem *self = BRISK_APPS_ITEM(item);

        const gchar *fields[] = {
                g_app_info_get_display_name(G_APP_INFO(self->info)),
                g_app_info_get_description(G_APP_INFO(self->info)),
                g_app_info_get_name(G_APP_INFO(self->info)),
                g_app_info_get_executable(G_APP_INFO(self->info)),
        };

        const gchar *const *keywords = g_desktop_app_info_get_keywords(self->info);

        if (brisk_apps_array_contains(fields, G_N_ELEMENTS(fields), term)) {
                return TRUE;
        }

        if (!keywords) {
                return FALSE;
        }

        return brisk_apps_array_contains((const gchar **)keywords,
                                         g_strv_length((gchar **)keywords),
                                         term);
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
