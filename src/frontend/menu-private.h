/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016-2018 Brisk Menu Developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "backend/backend.h"
#include "entry-button.h"
#include "key-binder.h"
#include "launcher.h"
#include "libsaver-glue.h"
#include "libsession-glue.h"
#include "menu-window.h"
#include "util.h"
#include <gtk/gtk.h>

typedef enum {
        SEARCH_POS_MIN = 0,
        SEARCH_POS_BOTTOM = 1 << 0,
        SEARCH_POS_TOP = 1 << 1,
        SEARCH_POS_AUTOMATIC = 1 << 2,
        SEARCH_POS_MAX,
} SearchPosition;

struct _BriskMenuWindowClass {
        GtkWindowClass parent_class;

        const gchar *(*get_id)(BriskMenuWindow *);
        const gchar *(*get_display_name)(BriskMenuWindow *);
        void (*update_screen_position)(BriskMenuWindow *);
        void (*update_search)(BriskMenuWindow *);
        void (*add_item)(BriskMenuWindow *, BriskItem *, BriskBackend *);
        void (*add_section)(BriskMenuWindow *, BriskSection *, BriskBackend *);
        void (*invalidate_filter)(BriskMenuWindow *, BriskBackend *);
        void (*reset)(BriskMenuWindow *, BriskBackend *);

        gpointer padding[12];
};

/**
 * BriskMenuWindow is an abstract top-level class which is used as the base
 * of all other window implementations within Brisk.
 */
struct _BriskMenuWindow {
        GtkWindow parent;
        /* Current panel position */
        GtkPositionType position;

        /* Current search position */
        SearchPosition search_position;

        /* Top search entry */
        GtkWidget *search;

        /* Whether we have a grab or not */
        gboolean grabbed;

        /* Whether we're in rollover mode or not */
        gboolean rollover;

        /* Global settings for all BriskMenu instances */
        GSettings *settings;

        /* Control hotkeys */
        BriskKeyBinder *binder;
        gchar *shortcut;

        GtkWidget *context_menu;
        GActionGroup *context_group;

        /* Each backend gets its own box in the sidebar */
        GHashTable *section_boxes;

        /* Each backend is also plugged into one big map */
        GHashTable *backends;

        /* Acknowledge a single ID "contains" map */
        GHashTable *item_store;

        /* Control launches */
        BriskMenuLauncher *launcher;

        /* Search term, may be null at any point. Used for filtering */
        gchar *search_term;

        /* The current section used in filtering */
        BriskSection *active_section;

        gboolean filtering;

        GtkWidget *relative_to;

        /* The widget that contains our backend section boxes */
        GtkWidget *section_box_holder;

        GtkWidget *section_box_leader;

        /* Session management */
        GnomeSessionManager *session;
        MateScreenSaver *saver;
};

GtkWidget *brisk_menu_window_get_section_box(BriskMenuWindow *self, BriskBackend *backend);

/**
 * Update internal notion of where the parent panel is on screen
 */
void brisk_menu_window_set_parent_position(BriskMenuWindow *window, GtkPositionType position);
void brisk_menu_window_select_sections(BriskMenuWindow *self);
GtkWidget *brisk_menu_window_find_first_visible_radio(BriskMenuWindow *self);

/* Loader */
gboolean brisk_menu_window_load_menus(BriskMenuWindow *self);
void brisk_menu_window_init_backends(BriskMenuWindow *self);
void brisk_menu_window_remove_category(GtkWidget *widget, BriskMenuWindow *self);

/* Sorting */
gint brisk_menu_window_sort(BriskMenuWindow *self, BriskItem *itemA, BriskItem *itemB);

/* Keyboard */
gboolean brisk_menu_window_key_press(BriskMenuWindow *self, GdkEvent *event, gpointer v);
gboolean brisk_menu_window_key_release(BriskMenuWindow *self, GdkEvent *event, gpointer v);
void brisk_menu_window_update_hotkey(BriskMenuWindow *self, gchar *key);

/* Global grabs */
void brisk_menu_window_configure_grabs(BriskMenuWindow *self);

/* Session controls */
void brisk_menu_window_logout(BriskMenuWindow *self, gpointer v);
void brisk_menu_window_shutdown(BriskMenuWindow *self, gpointer v);
void brisk_menu_window_lock(BriskMenuWindow *self, gpointer v);
gboolean brisk_menu_window_setup_session(BriskMenuWindow *self);

/* Settings */
void brisk_menu_window_init_settings(BriskMenuWindow *self);
void brisk_menu_window_pump_settings(BriskMenuWindow *self);

/* Context menu */
void brisk_menu_window_show_context(BriskMenuWindow *self, BriskItem *item,
                                    BriskMenuEntryButton *button);
void brisk_menu_window_configure_context(BriskMenuWindow *self);

/* Search */
void brisk_menu_window_clear_search(GtkEntry *entry, GtkEntryIconPosition pos, GdkEvent *event,
                                    gpointer v);
void brisk_menu_window_search(BriskMenuWindow *self, GtkEntry *entry);
gboolean brisk_menu_window_filter_apps(BriskMenuWindow *self, GtkWidget *child);

DEF_AUTOFREE(GtkWidget, gtk_widget_destroy)
DEF_AUTOFREE(GSList, g_slist_free)
DEF_AUTOFREE(GList, g_list_free)
DEF_AUTOFREE(GFile, g_object_unref)
DEF_AUTOFREE(GIcon, g_object_unref)
DEF_AUTOFREE(gchar, g_free)
DEF_AUTOFREE(GdkAppLaunchContext, g_object_unref)
DEF_AUTOFREE(GError, g_error_free)
DEF_AUTOFREE(GAppInfo, g_object_unref)

/* Helper for gsettings */
typedef gchar *gstrv;
DEF_AUTOFREE(gstrv, g_strfreev)

/**
 * Convenience function to remove children from a container
 */
__attribute__((always_inline)) static inline void brisk_menu_kill_children(GtkContainer *container)
{
        for (autofree(GList) *elem = gtk_container_get_children(container); elem;
             elem = elem->next) {
                gtk_widget_destroy(GTK_WIDGET(elem->data));
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
