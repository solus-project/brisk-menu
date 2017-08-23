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
#include <glib/gi18n.h>
#include <glib/gstdio.h>
BRISK_END_PEDANTIC

#include <errno.h>
#include <string.h>

DEF_AUTOFREE(GFile, g_object_unref)
DEF_AUTOFREE(gchar, g_free)
DEF_AUTOFREE(GError, g_error_free)

typedef enum {
        PIN_STATUS_UNPINNABLE = 0,
        PIN_STATUS_PINNED = 1,
        PIN_STATUS_UNPINNED = 2,
} DesktopPinStatus;

/**
 * get_desktop_item_source:
 *
 * Get a GFile for the BriskItem's source
 */
static GFile *get_desktop_item_source(BriskItem *item)
{
        autofree(gchar) *uri = NULL;

        if (!item) {
                return NULL;
        }

        uri = brisk_item_get_uri(item);
        if (!uri) {
                return NULL;
        }
        return g_file_new_for_uri(uri);
}

/**
 * get_item_path will return the path that would be pinned on the desktop
 */
static gchar *get_desktop_item_target_path(GFile *file)
{
        if (!g_file_query_exists(file, NULL)) {
                return NULL;
        }
        autofree(gchar) *basename = g_file_get_basename(file);

        return g_build_path(G_DIR_SEPARATOR_S,
                            g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP),
                            basename,
                            NULL);
}

/**
 * get_desktop_item_target:
 *
 * Get the target GFile on the desktop
 */
static GFile *get_desktop_item_target(GFile *file)
{
        autofree(gchar) *path = NULL;

        path = get_desktop_item_target_path(file);
        return g_file_new_for_path(path);
}

/**
 * brisk_favourites_backend_action_desktop_pin will pin the item to the desktop
 * by copying the source file to the target
 */
static void brisk_favourites_backend_action_desktop_pin(__brisk_unused__ GSimpleAction *action,
                                                        __brisk_unused__ GVariant *parameter,
                                                        BriskFavouritesBackend *self)
{
        autofree(GFile) *source = NULL;
        autofree(GFile) *dest = NULL;
        autofree(GError) *error = NULL;
        autofree(gchar) *path = NULL;

        source = get_desktop_item_source(self->active_item);
        if (!source) {
                return;
        }
        dest = get_desktop_item_target(source);
        if (!dest) {
                return;
        }

        path = g_file_get_path(dest);
        if (!path) {
                return;
        }

        /* Consider making async. */
        if (!g_file_copy(source,
                         dest,
                         G_FILE_COPY_ALL_METADATA | G_FILE_COPY_OVERWRITE,
                         NULL,
                         NULL,
                         NULL,
                         &error)) {
                /* Consider using libnotify */
                g_message("Failed to pin desktop item: %s", error->message);
        }

        /* MATE will sanitize .desktop files that are chmod +x */
        int r = g_chmod(path, 00755);
        if (r != 0) {
                g_message("Failed to chmod desktop item: %s", strerror(errno));
        }
}

/**
 * brisk_favourites_backend_action_desktop_unpin will attempt to unpin the item
 * from the desktop by removing the .desktop file
 */
static void brisk_favourites_backend_action_desktop_unpin(__brisk_unused__ GSimpleAction *action,
                                                          __brisk_unused__ GVariant *parameter,
                                                          BriskFavouritesBackend *self)
{
        autofree(GFile) *source = NULL;
        autofree(GFile) *dest = NULL;
        autofree(GError) *error = NULL;

        source = get_desktop_item_source(self->active_item);
        if (!source) {
                return;
        }

        dest = get_desktop_item_target(source);
        if (!dest || !g_file_query_exists(dest, NULL)) {
                return;
        }

        if (!g_file_delete(dest, NULL, &error)) {
                /* Consider using libnotify */
                g_message("Unable to unpin desktop item: %s", error->message);
        }
}

/**
 * brisk_favourites_backend_is_desktop_pinned:
 *
 * Determine if the source file is actually pinned to the desktop or not
 */
static DesktopPinStatus brisk_favourites_backend_get_desktop_pin_status(BriskItem *item)
{
        autofree(GFile) *source = NULL;
        autofree(GFile) *dest = NULL;
        autofree(gchar) *base = NULL;

        source = get_desktop_item_source(item);
        if (!source) {
                return PIN_STATUS_UNPINNABLE;
        }

        /* .desktop .. */
        base = g_file_get_basename(source);
        if (!g_str_has_suffix(base, ".desktop")) {
                return PIN_STATUS_UNPINNABLE;
        }

        dest = get_desktop_item_target(source);
        if (!dest || !g_file_query_exists(dest, NULL)) {
                return PIN_STATUS_UNPINNED;
        }

        return PIN_STATUS_PINNED;
}

/**
 * brisk_favourites_backend_init_desktop:
 *
 * Initialise actions for the favourite backend's .desktop functionality
 */
void brisk_favourites_backend_init_desktop(BriskFavouritesBackend *self)
{
        self->action_add_desktop = g_simple_action_new("favourites.pin-desktop", NULL);
        g_signal_connect(self->action_add_desktop,
                         "activate",
                         G_CALLBACK(brisk_favourites_backend_action_desktop_pin),
                         self);
        self->action_remove_desktop = g_simple_action_new("favourites.unpin-desktop", NULL);
        g_signal_connect(self->action_remove_desktop,
                         "activate",
                         G_CALLBACK(brisk_favourites_backend_action_desktop_unpin),
                         self);
}

/**
 * brisk_favourites_backend_menu_desktop:
 *
 * Add relevant entries to the context menu pertaining to .desktop handling
 */
void brisk_favourites_backend_menu_desktop(BriskFavouritesBackend *self, GMenu *menu,
                                           GActionGroup *group)
{
        g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(self->action_add_desktop));
        g_action_map_add_action(G_ACTION_MAP(group), G_ACTION(self->action_remove_desktop));
        DesktopPinStatus t = brisk_favourites_backend_get_desktop_pin_status(self->active_item);

        switch (t) {
        case PIN_STATUS_PINNED:
                g_menu_append(menu,
                              _("Unpin from desktop"),
                              "brisk-context-items.favourites.unpin-desktop");
                break;
        case PIN_STATUS_UNPINNED:
                g_menu_append(menu,
                              _("Pin to desktop"),
                              "brisk-context-items.favourites.pin-desktop");
                break;
        case PIN_STATUS_UNPINNABLE:
        default:
                return;
        }
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
