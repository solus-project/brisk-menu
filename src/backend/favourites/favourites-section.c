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

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "favourites-section.h"
#include <gio/gio.h>
#include <glib/gi18n.h>
BRISK_END_PEDANTIC

struct _BriskFavouritesSectionClass {
        BriskSectionClass parent_class;
};

struct _BriskFavouritesSection {
        BriskSection parent;
        GIcon *icon; /**<Display icon */
        GSettings *settings;
};

G_DEFINE_TYPE(BriskFavouritesSection, brisk_favourites_section, BRISK_TYPE_SECTION)

/* Helper for gsettings */
typedef gchar *gstrv;
DEF_AUTOFREE(gstrv, g_strfreev)

/**
 * Basic subclassing
 */
static const gchar *brisk_favourites_section_get_id(BriskSection *item);
static const gchar *brisk_favourites_section_get_name(BriskSection *item);
static const GIcon *brisk_favourites_section_get_icon(BriskSection *item);
static const gchar *brisk_favourites_section_get_backend_id(BriskSection *item);
static gboolean brisk_favourites_section_can_show_item(BriskSection *section, BriskItem *item);

/**
 * brisk_favourites_section_dispose:
 *
 * Clean up a BriskFavouritesSection instance
 */
static void brisk_favourites_section_dispose(GObject *obj)
{
        BriskFavouritesSection *self = BRISK_FAVOURITES_SECTION(obj);

        g_clear_object(&self->icon);
        g_clear_object(&self->settings);

        G_OBJECT_CLASS(brisk_favourites_section_parent_class)->dispose(obj);
}

/**
 * brisk_favourites_section_class_init:
 *
 * Handle class initialisation
 */
static void brisk_favourites_section_class_init(BriskFavouritesSectionClass *klazz)
{
        BriskSectionClass *s_class = BRISK_SECTION_CLASS(klazz);
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* item vtable hookup */
        s_class->get_id = brisk_favourites_section_get_id;
        s_class->get_name = brisk_favourites_section_get_name;
        s_class->get_icon = brisk_favourites_section_get_icon;
        s_class->get_backend_id = brisk_favourites_section_get_backend_id;
        s_class->can_show_item = brisk_favourites_section_can_show_item;

        obj_class->dispose = brisk_favourites_section_dispose;
}

/**
 * brisk_favourites_section_init:
 *
 * Handle construction of the BriskFavouritesSection. Does absolutely nothing
 * special outside of creating our icon.
 */
static void brisk_favourites_section_init(BriskFavouritesSection *self)
{
        self->icon = g_themed_icon_new_with_default_fallbacks("emblem-favorite");
        self->settings = g_settings_new("com.solus-project.brisk-menu");
}

static gboolean brisk_favourites_section_get_item_is_pinned(BriskSection *section, BriskItem *item)
{
        autofree(gstrv) *favourites = NULL;

        BriskFavouritesSection *self = BRISK_FAVOURITES_SECTION(section);
        favourites = g_settings_get_strv(self->settings, "favourites");

        if (!favourites) {
                return FALSE;
        }

        return g_strv_contains((const gchar *const *)favourites, brisk_item_get_id(item));
}

static const gchar *brisk_favourites_section_get_id(__brisk_unused__ BriskSection *section)
{
        return "favourites";
}

static const gchar *brisk_favourites_section_get_name(__brisk_unused__ BriskSection *section)
{
        return _("Favourites");
}

static const GIcon *brisk_favourites_section_get_icon(BriskSection *section)
{
        BriskFavouritesSection *self = BRISK_FAVOURITES_SECTION(section);
        return (const GIcon *)self->icon;
}

static const gchar *brisk_favourites_section_get_backend_id(__brisk_unused__ BriskSection *item)
{
        return "favourites";
}

static gboolean brisk_favourites_section_can_show_item(__brisk_unused__ BriskSection *section,
                                                       __brisk_unused__ BriskItem *item)
{
        return brisk_favourites_section_get_item_is_pinned(section, item);
}

/**
 * brisk_favourites_section_new:
 *
 * Return a new BriskFavouritesSection
 */
BriskSection *brisk_favourites_section_new()
{
        return g_object_new(BRISK_TYPE_FAVOURITES_SECTION, NULL);
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
