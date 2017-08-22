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
        GHashTable *favourites;

        /* Action management */
        BriskItem *active_item;
        GSimpleAction *action_remove;
        GSimpleAction *action_add;
};

G_DEFINE_TYPE(BriskFavouritesBackend, brisk_favourites_backend, BRISK_TYPE_BACKEND)

/* Helper for gsettings */
typedef gchar *gstrv;
DEF_AUTOFREE(gstrv, g_strfreev)

static gboolean brisk_favourites_backend_load(BriskBackend *backend);
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

static GMenu *brisk_favourites_backend_get_item_actions(BriskBackend *backend, BriskItem *item,
                                                        GActionGroup *group)
{
        GMenu *ret = NULL;
        BriskFavouritesBackend *self = BRISK_FAVOURITES_BACKEND(backend);

        g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(self->action_add));
        g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(self->action_remove));

        self->active_item = item;

        ret = g_menu_new();

        if (brisk_favourites_backend_is_pinned(self, item)) {
                g_menu_append(ret,
                              _("Remove from favourites"),
                              "brisk-context-items.favourites.unpin");
        } else {
                g_menu_append(ret, _("Add to favourites"), "brisk-context-items.favourites.pin");
        }

        return ret;
}

/**
 * brisk_favourites_backend_dispose:
 *
 * Clean up a BriskFavouritesBackend instance
 */
static void brisk_favourites_backend_dispose(GObject *obj)
{
        BriskFavouritesBackend *self = BRISK_FAVOURITES_BACKEND(obj);
        g_clear_object(&self->action_add);
        g_clear_object(&self->action_remove);
        g_clear_object(&self->settings);
        g_clear_pointer(&self->favourites, g_hash_table_unref);
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
 * Handle changes to the favourites schema. We'll reset our table and restore the
 * entries and retain the order within the list, which will come in useful in
 * future when we want to enable reordering of entries.
 */
static void brisk_favourites_backend_changed(GSettings *settings, const gchar *key,
                                             BriskFavouritesBackend *self)
{
        autofree(gstrv) *favs = g_settings_get_strv(settings, key);
        g_hash_table_remove_all(self->favourites);

        if (!favs) {
                return;
        }

        for (guint i = 0; i < g_strv_length(favs); i++) {
                g_hash_table_insert(self->favourites, g_strdup(favs[i]), GUINT_TO_POINTER(i));
        }
}

/**
 * brisk_favourites_backend_init:
 *
 * Handle construction of the BriskFavouritesBackend
 */
static void brisk_favourites_backend_init(__brisk_unused__ BriskFavouritesBackend *self)
{
        self->settings = g_settings_new("com.solus-project.brisk-menu");
        g_signal_connect(self->settings,
                         "changed::favourites",
                         G_CALLBACK(brisk_favourites_backend_changed),
                         self);

        self->action_add = g_simple_action_new("favourites.pin", NULL);
        g_signal_connect(self->action_add,
                         "activate",
                         G_CALLBACK(brisk_favourites_backend_pin_item),
                         self);
        self->action_remove = g_simple_action_new("favourites.unpin", NULL);
        g_signal_connect(self->action_remove,
                         "activate",
                         G_CALLBACK(brisk_favourites_backend_unpin_item),
                         self);

        /* Allow O(1) lookup for the "is pinned" logic */
        self->favourites = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

        /* Force load of the backend pinned items */
        brisk_favourites_backend_changed(self->settings, "favourites", self);
}

/**
 * brisk_favourites_backend_is_pinned:
 *
 * Determine whether the BriskItem is registered as pinned
 */
gboolean brisk_favourites_backend_is_pinned(BriskFavouritesBackend *self, BriskItem *item)
{
        if (!item || !self) {
                return FALSE;
        }

        const gchar *id = brisk_item_get_id(item);
        return g_hash_table_contains(self->favourites, id);
}

/**
 * brisk_favourites_backend_load:
 *
 * On load we just emit a new stock section item
 */
static gboolean brisk_favourites_backend_load(BriskBackend *backend)
{
        BriskFavouritesBackend *self = BRISK_FAVOURITES_BACKEND(backend);
        brisk_backend_section_added(backend, brisk_favourites_section_new(self));
        return TRUE;
}

static void brisk_favourites_backend_pin_item(__brisk_unused__ GSimpleAction *action,
                                              __brisk_unused__ GVariant *parameter,
                                              BriskFavouritesBackend *self)
{
        gstrv *old = NULL;
        autofree(gstrv) *new = NULL;
        guint size;

        if (!self->active_item) {
                return;
        }

        const gchar *item_id = brisk_item_get_id(self->active_item);
        self->active_item = NULL;

        old = g_settings_get_strv(self->settings, "favourites");
        /* Increase l+2 for new item and NULL terminator */
        size = g_strv_length(old) + 2;

        new = g_realloc_n(old, size, sizeof(gchar *));

        new[size - 2] = g_strdup(item_id);
        new[size - 1] = NULL;

        g_settings_set_strv(self->settings, "favourites", (const gchar **)new);
}

static void brisk_favourites_backend_unpin_item(__brisk_unused__ GSimpleAction *action,
                                                __brisk_unused__ GVariant *parameter,
                                                BriskFavouritesBackend *self)
{
        GArray *array;
        autofree(gstrv) *old = NULL;
        gint i;

        if (!self->active_item) {
                return;
        }

        const gchar *item_id = brisk_item_get_id(self->active_item);
        self->active_item = NULL;

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
