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
#include "menu-private.h"
BRISK_END_PEDANTIC

G_DEFINE_TYPE(BriskMenuWindow, brisk_menu_window, GTK_TYPE_WINDOW)

static void brisk_menu_window_set_property(GObject *object, guint id, const GValue *value,
                                           GParamSpec *spec);
static void brisk_menu_window_get_property(GObject *object, guint id, GValue *value,
                                           GParamSpec *spec);
enum { PROP_RELATIVE_TO = 1, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

/**
 * brisk_menu_window_dispose:
 *
 * Clean up a BriskMenuWindow instance
 */
static void brisk_menu_window_dispose(GObject *obj)
{
        BriskMenuWindow *self = BRISK_MENU_WINDOW(obj);

        g_clear_object(&self->binder);
        g_clear_pointer(&self->shortcut, g_free);
        g_clear_pointer(&self->search_term, g_free);
        g_clear_object(&self->launcher);
        g_clear_object(&self->session);
        g_clear_object(&self->saver);
        g_clear_object(&self->settings);
        g_clear_pointer(&self->item_store, g_hash_table_unref);
        g_clear_pointer(&self->section_boxes, g_hash_table_unref);
        g_clear_pointer(&self->backends, g_hash_table_unref);
        g_clear_pointer(&self->context_menu, gtk_widget_destroy);
        g_clear_object(&self->context_group);

        G_OBJECT_CLASS(brisk_menu_window_parent_class)->dispose(obj);
}

/**
 * brisk_menu_window_class_init:
 *
 * Handle class initialisation
 */
static void brisk_menu_window_class_init(BriskMenuWindowClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_menu_window_dispose;
        obj_class->set_property = brisk_menu_window_set_property;
        obj_class->get_property = brisk_menu_window_get_property;

        /* Set up properties */
        obj_properties[PROP_RELATIVE_TO] =
            g_param_spec_pointer("relative-to",
                                 "GtkWidget we appear near",
                                 "Owning GtkWidget for this BriskMenuWindow",
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

/**
 * brisk_menu_window_init:
 *
 * Handle construction of the BriskMenuWindow
 */
static void brisk_menu_window_init(BriskMenuWindow *self)
{
        gtk_window_set_decorated(GTK_WINDOW(self), FALSE);
        gtk_window_set_type_hint(GTK_WINDOW(self), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
        gtk_window_set_skip_pager_hint(GTK_WINDOW(self), TRUE);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(self), TRUE);

        /* Initialise main tables */
        self->item_store = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
        self->section_boxes = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
        self->backends = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_object_unref);

        self->binder = brisk_key_binder_new();
        self->launcher = brisk_menu_launcher_new();

        brisk_menu_window_init_settings(self);
}

static void brisk_menu_window_set_property(GObject *object, guint id, const GValue *value,
                                           GParamSpec *spec)
{
        BriskMenuWindow *self = BRISK_MENU_WINDOW(object);

        switch (id) {
        case PROP_RELATIVE_TO:
                self->relative_to = g_value_get_pointer(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void brisk_menu_window_get_property(GObject *object, guint id, GValue *value,
                                           GParamSpec *spec)
{
        BriskMenuWindow *self = BRISK_MENU_WINDOW(object);

        switch (id) {
        case PROP_RELATIVE_TO:
                g_value_set_pointer(value, self->relative_to);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

void brisk_menu_window_set_parent_position(BriskMenuWindow *self, GtkPositionType position)
{
        self->position = position;
        brisk_menu_window_update_screen_position(self);
        brisk_menu_window_update_search(self);
}

/**
 * brisk_menu_window_select_sidebar:
 *
 * Select the first child in the section box holder prior to any kind of display
 */
void brisk_menu_window_select_sections(BriskMenuWindow *self)
{
        GtkWidget *next_child = NULL;

        /* Activate the first child in the group after the dummy widget */
        next_child = brisk_menu_window_find_first_visible_radio(self);
        if (next_child) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(next_child), TRUE);
        }
}

GtkWidget *brisk_menu_window_find_first_visible_radio(BriskMenuWindow *self)
{
        autofree(GList) *box_kids = NULL;
        autofree(GList) *box_kids2 = NULL;
        GtkWidget *main_box = NULL;
        GList *elem = NULL;

        box_kids = gtk_container_get_children(GTK_CONTAINER(self->section_box_holder));
        for (elem = box_kids; elem; elem = elem->next) {
                if (!GTK_IS_BOX(elem->data)) {
                        continue;
                }
                main_box = elem->data;
                break;
        }
        if (!main_box) {
                return NULL;
        }
        box_kids2 = gtk_container_get_children(GTK_CONTAINER(main_box));
        return g_list_nth_data(box_kids2, 0);
}

/**
 * brisk_menu_window_get_id:
 *
 * Return the unique ID for the window
 * @note This string is owned by the window and must not be freed
 */
const gchar *brisk_menu_window_get_id(BriskMenuWindow *window)
{
        g_assert(window != NULL);
        BriskMenuWindowClass *klazz = BRISK_MENU_WINDOW_GET_CLASS(window);
        g_assert(klazz->get_id != NULL);
        return klazz->get_id(window);
}

/**
 * brisk_menu_window_get_display_name:
 *
 * Return the display name for the window
 * @note This string is owned by the window and must not be freed
 */
const gchar *brisk_menu_window_get_display_name(BriskMenuWindow *window)
{
        g_assert(window != NULL);
        BriskMenuWindowClass *klazz = BRISK_MENU_WINDOW_GET_CLASS(window);
        g_assert(klazz->get_display_name != NULL);
        return klazz->get_display_name(window);
}

/**
 * brisk_menu_window_update_screen_position:
 *
 * Ask that the menu window updates it's position on screen
 */
void brisk_menu_window_update_screen_position(BriskMenuWindow *window)
{
        g_assert(window != NULL);
        BriskMenuWindowClass *klazz = BRISK_MENU_WINDOW_GET_CLASS(window);
        g_assert(klazz->add_item != NULL);
        klazz->update_screen_position(window);
}

void brisk_menu_window_update_search(BriskMenuWindow *window)
{
        g_assert(window != NULL);
        BriskMenuWindowClass *klazz = BRISK_MENU_WINDOW_GET_CLASS(window);
        if (klazz->update_search) {
                klazz->update_search(window);
        }
}

void brisk_menu_window_add_item(BriskMenuWindow *window, BriskItem *item, BriskBackend *backend)
{
        g_assert(window != NULL);
        BriskMenuWindowClass *klazz = BRISK_MENU_WINDOW_GET_CLASS(window);
        g_assert(klazz->add_item != NULL);
        klazz->add_item(window, item, backend);
}

void brisk_menu_window_add_section(BriskMenuWindow *window, BriskSection *section,
                                   BriskBackend *backend)
{
        g_assert(window != NULL);
        BriskMenuWindowClass *klazz = BRISK_MENU_WINDOW_GET_CLASS(window);
        g_assert(klazz->add_section != NULL);
        klazz->add_section(window, section, backend);
}

void brisk_menu_window_invalidate_filter(BriskMenuWindow *window, BriskBackend *backend)
{
        g_assert(window != NULL);
        BriskMenuWindowClass *klazz = BRISK_MENU_WINDOW_GET_CLASS(window);
        g_assert(klazz->invalidate_filter != NULL);
        klazz->invalidate_filter(window, backend);
}

void brisk_menu_window_reset(BriskMenuWindow *window, BriskBackend *backend)
{
        g_assert(window != NULL);
        BriskMenuWindowClass *klazz = BRISK_MENU_WINDOW_GET_CLASS(window);
        g_assert(klazz->reset != NULL);
        klazz->reset(window, backend);
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
