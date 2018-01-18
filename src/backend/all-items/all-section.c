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

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "all-section.h"
#include <gio/gio.h>
#include <glib/gi18n.h>
BRISK_END_PEDANTIC

struct _BriskAllItemsSectionClass {
        BriskSectionClass parent_class;
};

/**
 * BriskAllItemsSection is an incredibly simple backend that simple adds a single
 * section to the sidebar, and is allowed to display any item when the category
 * is active.
 *
 * It also serves as a great starting point for anyone wishing to implement their
 * own backends.
 */
struct _BriskAllItemsSection {
        BriskSection parent;
        GIcon *icon; /**<Display icon */
};

G_DEFINE_TYPE(BriskAllItemsSection, brisk_all_items_section, BRISK_TYPE_SECTION)

/**
 * Basic subclassing
 */
static const gchar *brisk_all_items_section_get_id(BriskSection *item);
static const gchar *brisk_all_items_section_get_name(BriskSection *item);
static const GIcon *brisk_all_items_section_get_icon(BriskSection *item);
static const gchar *brisk_all_items_section_get_backend_id(BriskSection *item);
static gboolean brisk_all_items_section_can_show_item(BriskSection *section, BriskItem *item);

/**
 * brisk_all_items_section_dispose:
 *
 * Clean up a BriskAllItemsSection instance
 */
static void brisk_all_items_section_dispose(GObject *obj)
{
        BriskAllItemsSection *self = BRISK_ALL_ITEMS_SECTION(obj);

        g_clear_object(&self->icon);

        G_OBJECT_CLASS(brisk_all_items_section_parent_class)->dispose(obj);
}

/**
 * brisk_all_items_section_class_init:
 *
 * Handle class initialisation
 */
static void brisk_all_items_section_class_init(BriskAllItemsSectionClass *klazz)
{
        BriskSectionClass *s_class = BRISK_SECTION_CLASS(klazz);
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* item vtable hookup */
        s_class->get_id = brisk_all_items_section_get_id;
        s_class->get_name = brisk_all_items_section_get_name;
        s_class->get_icon = brisk_all_items_section_get_icon;
        s_class->get_backend_id = brisk_all_items_section_get_backend_id;
        s_class->can_show_item = brisk_all_items_section_can_show_item;

        obj_class->dispose = brisk_all_items_section_dispose;
}

/**
 * brisk_all_items_section_init:
 *
 * Handle construction of the BriskAllItemsSection. Does absolutely nothing
 * special outside of creating our icon.
 */
static void brisk_all_items_section_init(BriskAllItemsSection *self)
{
        self->icon = g_themed_icon_new_with_default_fallbacks("starred");
}

static const gchar *brisk_all_items_section_get_id(__brisk_unused__ BriskSection *section)
{
        return "all-items:main";
}

static const gchar *brisk_all_items_section_get_name(__brisk_unused__ BriskSection *section)
{
        return _("All");
}

static const GIcon *brisk_all_items_section_get_icon(BriskSection *section)
{
        BriskAllItemsSection *self = BRISK_ALL_ITEMS_SECTION(section);
        return (const GIcon *)self->icon;
}

static const gchar *brisk_all_items_section_get_backend_id(__brisk_unused__ BriskSection *item)
{
        return "all-items";
}

/**
 * Being the magical "all" filter, we return TRUE for everything.
 * Cuz we can show them all. :3
 */
static gboolean brisk_all_items_section_can_show_item(__brisk_unused__ BriskSection *section,
                                                      __brisk_unused__ BriskItem *item)
{
        return TRUE;
}

/**
 * brisk_all_items_section_new:
 *
 * Return a new BriskAllItemsSection
 */
BriskSection *brisk_all_items_section_new()
{
        return g_object_new(BRISK_TYPE_ALL_ITEMS_SECTION, NULL);
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
