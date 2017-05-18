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
#include "apps-section.h"
#include <gio/gdesktopappinfo.h>
BRISK_END_PEDANTIC

enum { PROP_DIRECTORY = 1, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

struct _BriskAppsSectionClass {
        BriskSectionClass parent_class;
};

/**
 * BriskAppsSection is a type of item for the Brisk menu which is backed by
 * a .desktop file
 */
struct _BriskAppsSection {
        BriskSection parent;

        gchar *id;
        gchar *name;
        GIcon *icon;
};

G_DEFINE_TYPE(BriskAppsSection, brisk_apps_section, BRISK_TYPE_SECTION)

DEF_AUTOFREE(GFile, g_object_unref)

/**
 * Basic subclassing
 */
static const gchar *brisk_apps_section_get_id(BriskSection *item);
static const gchar *brisk_apps_section_get_name(BriskSection *item);
static const GIcon *brisk_apps_section_get_icon(BriskSection *item);
static const gchar *brisk_apps_section_get_backend_id(BriskSection *item);
static gboolean brisk_apps_section_can_show_item(BriskSection *section, BriskItem *item);

/**
 * Create a GIcon for the given path
 */
static GIcon *brisk_apps_section_create_path_icon(const gchar *path)
{
        autofree(GFile) *file = NULL;

        file = g_file_new_for_path(path);
        if (!file) {
                return NULL;
        }
        return g_file_icon_new(file);
}

static void brisk_apps_section_update_directory(BriskAppsSection *self,
                                                MateMenuTreeDirectory *directory)
{
        g_clear_object(&self->icon);
        g_clear_pointer(&self->id, g_free);
        g_clear_pointer(&self->name, g_free);
        const gchar *icon = NULL;

        if (!directory) {
                return;
        }

        /* Set our ID and name */
        self->id =
            g_strdup_printf("%s.mate-directory", matemenu_tree_directory_get_menu_id(directory));
        self->name = g_strdup(matemenu_tree_directory_get_name(directory));

        icon = matemenu_tree_directory_get_icon(directory);
        if (!icon) {
                return;
        }

        /* Set an appropriate icon based on the string */
        if (icon[0] == '/') {
                self->icon = brisk_apps_section_create_path_icon(icon);
        } else {
                self->icon = g_themed_icon_new_with_default_fallbacks(icon);
        }
}

static void brisk_apps_section_set_property(GObject *object, guint id, const GValue *value,
                                            GParamSpec *spec)
{
        BriskAppsSection *self = BRISK_APPS_SECTION(object);
        MateMenuTreeDirectory *directory = NULL;

        switch (id) {
        case PROP_DIRECTORY:
                directory = g_value_get_pointer(value);
                brisk_apps_section_update_directory(self, directory);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void brisk_apps_section_get_property(GObject *object, guint id, GValue *value,
                                            GParamSpec *spec)
{
        switch (id) {
        case PROP_DIRECTORY:
                /* We don't maintain the reference, we only want to use it in construction */
                g_value_set_pointer(value, NULL);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * brisk_apps_section_dispose:
 *
 * Clean up a BriskAppsSection instance
 */
static void brisk_apps_section_dispose(GObject *obj)
{
        BriskAppsSection *self = BRISK_APPS_SECTION(obj);

        g_clear_object(&self->icon);
        g_clear_pointer(&self->id, g_free);
        g_clear_pointer(&self->name, g_free);

        G_OBJECT_CLASS(brisk_apps_section_parent_class)->dispose(obj);
}

/**
 * brisk_apps_section_class_init:
 *
 * Handle class initialisation
 */
static void brisk_apps_section_class_init(BriskAppsSectionClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        BriskSectionClass *s_class = BRISK_SECTION_CLASS(klazz);

        /* item vtable hookup */
        s_class->get_id = brisk_apps_section_get_id;
        s_class->get_name = brisk_apps_section_get_name;
        s_class->get_icon = brisk_apps_section_get_icon;
        s_class->get_backend_id = brisk_apps_section_get_backend_id;
        s_class->can_show_item = brisk_apps_section_can_show_item;

        /* gobject vtable hookup */
        obj_class->dispose = brisk_apps_section_dispose;
        obj_class->set_property = brisk_apps_section_set_property;
        obj_class->get_property = brisk_apps_section_get_property;

        obj_properties[PROP_DIRECTORY] =
            g_param_spec_pointer("directory",
                                 "The MateMenuTreeDirectory",
                                 "Corresponding menu directory",
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

/**
 * brisk_apps_section_init:
 *
 * Handle construction of the BriskAppsSection
 */
static void brisk_apps_section_init(__brisk_unused__ BriskAppsSection *self)
{
}

static const gchar *brisk_apps_section_get_id(BriskSection *section)
{
        BriskAppsSection *self = BRISK_APPS_SECTION(section);
        return (const gchar *)self->id;
}

static const gchar *brisk_apps_section_get_name(BriskSection *section)
{
        BriskAppsSection *self = BRISK_APPS_SECTION(section);
        return (const gchar *)self->name;
}

static const GIcon *brisk_apps_section_get_icon(BriskSection *section)
{
        BriskAppsSection *self = BRISK_APPS_SECTION(section);
        return (const GIcon *)self->icon;
}

static const gchar *brisk_apps_section_get_backend_id(__brisk_unused__ BriskSection *item)
{
        return "apps";
}

/**
 * Long story short, if the section ID matches, we can show it.
 */
static gboolean brisk_apps_section_can_show_item(BriskSection *section, BriskItem *item)
{
        /* We only know how to handle our items */
        BriskAppsItem *apps_item = NULL;
        const gchar *section_id = NULL;
        BriskAppsSection *self = BRISK_APPS_SECTION(section);

        if (G_UNLIKELY(item == NULL) || G_UNLIKELY(!BRISK_IS_APPS_ITEM(item))) {
                return FALSE;
        }

        apps_item = BRISK_APPS_ITEM(item);
        section_id = brisk_apps_item_get_section_id(apps_item);
        if (section_id && g_str_equal(section_id, self->id)) {
                return TRUE;
        }
        return FALSE;
}

/**
 * brisk_apps_section_new:
 *
 * Return a new BriskAppsSection for the given menu directory.
 * @note: The directory will not be stored, it is used only to seed the
 * section.
 */
BriskSection *brisk_apps_section_new(MateMenuTreeDirectory *dir)
{
        return g_object_new(BRISK_TYPE_APPS_SECTION, "directory", dir, NULL);
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
