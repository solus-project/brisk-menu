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
#include "favourites-backend.h"
#include "favourites-section.h"
#include <glib/gi18n.h>
BRISK_END_PEDANTIC

struct _BriskFavouritesBackendClass {
        BriskBackendClass parent_class;
};

/**
 * BriskFavouritesBackend implements support for favourites in Brisk
 */
struct _BriskFavouritesBackend {
        BriskBackend parent;
        GSettings *settings;
};

G_DEFINE_TYPE(BriskFavouritesBackend, brisk_favourites_backend, BRISK_TYPE_BACKEND)

/* Helper for gsettings */
typedef gchar *gstrv;
DEF_AUTOFREE(gstrv, g_strfreev)

static gboolean brisk_favourites_backend_load(BriskBackend *backend);
static gboolean brisk_favourites_backend_get_item_pinned(BriskBackend *backend, BriskItem *item);
static void brisk_favourites_backend_pin_item(GSimpleAction *action, GVariant *parameter,
                                              BriskFavouritesBackend *self);
static void brisk_favourites_backend_unpin_item(GSimpleAction *action, GVariant *parameter,
                                                BriskFavouritesBackend *self);

/**
 * Tell the frontends what we are
 */
static unsigned int brisk_favourites_backend_get_flags(__brisk_unused__ BriskBackend *backend)
{
        return BRISK_BACKEND_SOURCE;
}

static const gchar *brisk_favourites_backend_get_id(__brisk_unused__ BriskBackend *backend)
{
        return "favourites";
}

static const gchar *brisk_favourites_backend_get_display_name(
    __brisk_unused__ BriskBackend *backend)
{
        return _("Favourites");
}

static GSList *brisk_favourites_backend_get_item_actions(BriskBackend *backend, BriskItem *item)
{
        GSList *list = NULL;

        BriskFavouritesBackend *self = BRISK_FAVOURITES_BACKEND(backend);

        GSimpleAction *action;
        if (brisk_favourites_backend_get_item_pinned(backend, item)) {
                action = g_simple_action_new(_("Remove from favourites"), NULL);
                g_signal_connect(action,
                                 "activate",
                                 G_CALLBACK(brisk_favourites_backend_unpin_item),
                                 self);
        } else {
                action = g_simple_action_new(_("Add to favourites"), NULL);
                g_signal_connect(action,
                                 "activate",
                                 G_CALLBACK(brisk_favourites_backend_pin_item),
                                 self);
        }

        g_object_set_data(G_OBJECT(action), "__brisk_item_id", (gpointer)brisk_item_get_id(item));

        list = g_slist_append(list, action);

        return list;
}

/**
 * brisk_favourites_backend_dispose:
 *
 * Clean up a BriskFavouritesBackend instance
 */
static void brisk_favourites_backend_dispose(GObject *obj)
{
        BriskFavouritesBackend *self = BRISK_FAVOURITES_BACKEND(obj);
        g_clear_object(&self->settings);
        G_OBJECT_CLASS(brisk_favourites_backend_parent_class)->dispose(obj);
}

/**
 * brisk_favourites_backend_class_init:
 *
 * Handle class initialisation
 */
static void brisk_favourites_backend_class_init(BriskFavouritesBackendClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        BriskBackendClass *b_class = BRISK_BACKEND_CLASS(klazz);

        /* Backend vtable hookup */
        b_class->get_flags = brisk_favourites_backend_get_flags;
        b_class->get_id = brisk_favourites_backend_get_id;
        b_class->get_display_name = brisk_favourites_backend_get_display_name;
        b_class->load = brisk_favourites_backend_load;
        b_class->get_item_actions = brisk_favourites_backend_get_item_actions;

        /* gobject vtable hookup */
        obj_class->dispose = brisk_favourites_backend_dispose;
}

/**
 * brisk_favourites_backend_init:
 *
 * Handle construction of the BriskFavouritesBackend
 */
static void brisk_favourites_backend_init(__brisk_unused__ BriskFavouritesBackend *self)
{
        self->settings = g_settings_new("com.solus-project.brisk-menu");
}

static gboolean brisk_favourites_backend_get_item_pinned(BriskBackend *backend, BriskItem *item)
{
        autofree(gstrv) *favourites = NULL;

        BriskFavouritesBackend *self = BRISK_FAVOURITES_BACKEND(backend);
        favourites = g_settings_get_strv(self->settings, "favourites");

        if (!favourites) {
                return FALSE;
        }

        return g_strv_contains((const gchar *const *)favourites, brisk_item_get_id(item));
}

/**
 * brisk_favourites_backend_load:
 *
 * On load we just emit a new stock section item
 */
static gboolean brisk_favourites_backend_load(BriskBackend *backend)
{
        brisk_backend_section_added(backend, brisk_favourites_section_new());
        return TRUE;
}

static void brisk_favourites_backend_pin_item(GSimpleAction *action,
                                              __brisk_unused__ GVariant *parameter,
                                              BriskFavouritesBackend *self)
{
        gstrv *old = NULL;
        autofree(gstrv) *new = NULL;
        guint size;

        const gchar *item_id = g_object_get_data(G_OBJECT(action), "__brisk_item_id");

        old = g_settings_get_strv(self->settings, "favourites");
        size = g_strv_length(old);

        size += 1; /* appended value */
        size += 1; /* NULL */

        new = g_realloc_n(old, size, sizeof(gchar *));

        new[size - 2] = g_strdup(item_id);
        new[size - 1] = NULL;

        g_settings_set_strv(self->settings, "favourites", (const gchar **)new);
}

static void brisk_favourites_backend_unpin_item(GSimpleAction *action,
                                                __brisk_unused__ GVariant *parameter,
                                                BriskFavouritesBackend *self)
{
        GArray *array;
        autofree(gstrv) *old = NULL;
        gint i;

        const gchar *item_id = g_object_get_data(G_OBJECT(action), "__brisk_item_id");

        old = g_settings_get_strv(self->settings, "favourites");
        array = g_array_new(TRUE, TRUE, sizeof(gchar *));

        for (i = 0; old[i] != NULL; i++) {
                if (!g_str_equal(old[i], item_id)) {
                        array = g_array_append_val(array, old[i]);
                }
        }

        g_settings_set_strv(self->settings, "favourites", (const gchar **)array->data);

        g_array_free(array, TRUE);
        brisk_backend_invalidate_filter(BRISK_BACKEND(self));
}

/**
 * brisk_favourites_backend_new:
 *
 * Return a newly created BriskFavouritesBackend
 */
BriskBackend *brisk_favourites_backend_new(void)
{
        return g_object_new(BRISK_TYPE_FAVOURITES_BACKEND, NULL);
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
