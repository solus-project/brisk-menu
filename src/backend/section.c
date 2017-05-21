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
#include "section.h"
BRISK_END_PEDANTIC

G_DEFINE_TYPE(BriskSection, brisk_section, G_TYPE_INITIALLY_UNOWNED)

/**
 * brisk_section_dispose:
 *
 * Clean up a BriskSection instance
 */
static void brisk_section_dispose(GObject *obj)
{
        G_OBJECT_CLASS(brisk_section_parent_class)->dispose(obj);
}

/**
 * brisk_section_class_init:
 *
 * Handle class initialisation
 */
static void brisk_section_class_init(BriskSectionClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_section_dispose;
}

/**
 * brisk_section_init:
 *
 * Handle construction of the BriskSection
 */
static void brisk_section_init(__brisk_unused__ BriskSection *self)
{
}

/**
 * brisk_section_get_id:
 *
 * Returns the unique ID for this section within the backend
 * @note This string belongs to the section, and must not be freed by the caller
 */
const gchar *brisk_section_get_id(BriskSection *section)
{
        g_assert(section != NULL);
        BriskSectionClass *klazz = BRISK_SECTION_GET_CLASS(section);
        g_assert(klazz->get_id != NULL);
        return klazz->get_id(section);
}

/**
 * brisk_section_get_name:
 *
 * Returns the name used when displaying the section in the menu
 * @note This string belongs to the section, and must not be freed by the caller
 */
const gchar *brisk_section_get_name(BriskSection *section)
{
        g_assert(section != NULL);
        BriskSectionClass *klazz = BRISK_SECTION_GET_CLASS(section);
        g_assert(klazz->get_name != NULL);
        return klazz->get_name(section);
}

/**
 * brisk_section_get_icon:
 *
 * Returns the Icon for this menu in the display.
 */
const GIcon *brisk_section_get_icon(BriskSection *section)
{
        g_assert(section != NULL);
        BriskSectionClass *klazz = BRISK_SECTION_GET_CLASS(section);
        if (!klazz->get_icon) {
                return NULL;
        }
        return klazz->get_icon(section);
}

/**
 * brisk_section_get_backend_id:
 *
 * Return the ID for the backend that owns this section
 */
const gchar *brisk_section_get_backend_id(BriskSection *section)
{
        g_assert(section != NULL);
        BriskSectionClass *klazz = BRISK_SECTION_GET_CLASS(section);
        g_assert(klazz->get_backend_id != NULL);
        return klazz->get_backend_id(section);
}

/**
 * brisk_section_can_show_item:
 *
 * Returns true if the items can be shown under this section
 */
gboolean brisk_section_can_show_item(BriskSection *section, BriskItem *item)
{
        g_assert(section != NULL);
        BriskSectionClass *klazz = BRISK_SECTION_GET_CLASS(section);
        if (!klazz->can_show_item) {
                return FALSE;
        }
        return klazz->can_show_item(section, item);
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
