/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016 Ikey Doherty <ikey@solus-project.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "entry-button.h"
#include "launcher.h"
#include "menu-private.h"
#include <gio/gdesktopappinfo.h>
#include <gtk/gtk.h>
#include <matemenu-tree.h>

BRISK_END_PEDANTIC

struct _BriskMenuEntryButtonClass {
        GtkButtonClass parent_class;
};

static void brisk_menu_entry_drag_begin(GtkWidget *widget, GdkDragContext *context);
static void brisk_menu_entry_drag_data(GtkWidget *widget, GdkDragContext *context,
                                       GtkSelectionData *data, guint info, guint time);
static void brisk_menu_entry_clicked(GtkButton *widget);

/**
 * BriskMenuEntryButton is the toplevel window type used within the applet.
 */
struct _BriskMenuEntryButton {
        GtkButton parent;
        MateMenuTreeEntry *entry;
        MateMenuTree *tree;
        GtkWidget *label;
        GtkWidget *image;
        GDesktopAppInfo *info;
        BriskMenuLauncher *launcher;
};

G_DEFINE_TYPE(BriskMenuEntryButton, brisk_menu_entry_button, GTK_TYPE_BUTTON)

enum { PROP_ENTRY = 1, PROP_TREE, PROP_LAUNCHER, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void brisk_menu_entry_button_set_property(GObject *object, guint id, const GValue *value,
                                                 GParamSpec *spec)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(object);

        switch (id) {
        case PROP_ENTRY:
                self->entry = g_value_get_pointer(value);
                break;
        case PROP_TREE:
                self->tree = g_value_get_pointer(value);
                break;
        case PROP_LAUNCHER:
                self->launcher = g_value_get_pointer(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void brisk_menu_entry_button_get_property(GObject *object, guint id, GValue *value,
                                                 GParamSpec *spec)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(object);

        switch (id) {
        case PROP_ENTRY:
                g_value_set_pointer(value, self->entry);
                break;
        case PROP_TREE:
                g_value_set_pointer(value, self->tree);
                break;
        case PROP_LAUNCHER:
                g_value_set_pointer(value, self->launcher);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * brisk_menu_entry_button_new:
 *
 * Construct a new BriskMenuEntryButton object
 */
GtkWidget *brisk_menu_entry_button_new(BriskMenuLauncher *launcher, MateMenuTree *tree,
                                       MateMenuTreeEntry *entry)
{
        return g_object_new(BRISK_TYPE_MENU_ENTRY_BUTTON,
                            "launcher",
                            launcher,
                            "tree",
                            tree,
                            "entry",
                            entry,
                            NULL);
}

/**
 * brisk_menu_entry_button_dispose:
 *
 * Clean up a BriskMenuEntryButton instance
 */
static void brisk_menu_entry_button_dispose(GObject *obj)
{
        BriskMenuEntryButton *self = NULL;

        self = BRISK_MENU_ENTRY_BUTTON(obj);
        g_clear_object(&self->info);

        G_OBJECT_CLASS(brisk_menu_entry_button_parent_class)->dispose(obj);
}

static GIcon *brisk_menu_entry_button_create_gicon(const char *path)
{
        autofree(GFile) *file = NULL;

        file = g_file_new_for_path(path);
        if (!file) {
                return NULL;
        }
        return g_file_icon_new(file);
}

/**
 * Handle constructor specifics for our button
 */
static void brisk_menu_entry_button_constructed(GObject *obj)
{
        const gchar *label = NULL;
        BriskMenuEntryButton *self = NULL;
        const gchar *icon_name = NULL;
        const gchar *desktop_id = NULL;

        self = BRISK_MENU_ENTRY_BUTTON(obj);

        /* matemenu has no gicon support, so do it ourselves. */
        icon_name = matemenu_tree_entry_get_icon(self->entry);
        if (icon_name && icon_name[0] == '/') {
                autofree(GIcon) *ico = brisk_menu_entry_button_create_gicon(icon_name);
                gtk_image_set_from_gicon(GTK_IMAGE(self->image), ico, GTK_ICON_SIZE_INVALID);
        } else {
                gtk_image_set_from_icon_name(GTK_IMAGE(self->image),
                                             icon_name,
                                             GTK_ICON_SIZE_INVALID);
        }
        gtk_image_set_pixel_size(GTK_IMAGE(self->image), 24);

        /* Determine our label based on the app */
        label = matemenu_tree_entry_get_name(self->entry);
        gtk_label_set_label(GTK_LABEL(self->label), label);
        gtk_widget_set_tooltip_text(GTK_WIDGET(self), matemenu_tree_entry_get_comment(self->entry));

        /* Load our .desktop info */
        desktop_id = matemenu_tree_entry_get_desktop_file_path(self->entry);
        if (desktop_id) {
                self->info = g_desktop_app_info_new_from_filename(desktop_id);
        }

        G_OBJECT_CLASS(brisk_menu_entry_button_parent_class)->constructed(obj);
}

/**
 * brisk_menu_entry_button_class_init:
 *
 * Handle class initialisation
 */
static void brisk_menu_entry_button_class_init(BriskMenuEntryButtonClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        GtkWidgetClass *wid_class = GTK_WIDGET_CLASS(klazz);
        GtkButtonClass *but_class = GTK_BUTTON_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_menu_entry_button_dispose;
        obj_class->set_property = brisk_menu_entry_button_set_property;
        obj_class->get_property = brisk_menu_entry_button_get_property;
        obj_class->constructed = brisk_menu_entry_button_constructed;

        /* widget vtable hookup */
        wid_class->drag_data_get = brisk_menu_entry_drag_data;
        wid_class->drag_begin = brisk_menu_entry_drag_begin;

        /* button vtable hookup */
        but_class->clicked = brisk_menu_entry_clicked;

        obj_properties[PROP_ENTRY] = g_param_spec_pointer("entry",
                                                          "The MateMenuTreeEntry",
                                                          "Entry that this launcher represents",
                                                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        obj_properties[PROP_TREE] = g_param_spec_pointer("tree",
                                                         "The MateMenuTree",
                                                         "Tree that this entry belongs to",
                                                         G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        obj_properties[PROP_LAUNCHER] = g_param_spec_pointer("launcher",
                                                             "The Brisk Launcher",
                                                             "Launcher used for starting apps",
                                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

/**
 * brisk_menu_entry_button_init:
 *
 * Handle construction of the BriskMenuEntryButton
 */
static void brisk_menu_entry_button_init(BriskMenuEntryButton *self)
{
        GtkStyleContext *style = NULL;
        GtkWidget *label = NULL;
        GtkWidget *image = NULL;
        GtkWidget *layout = NULL;

        static const GtkTargetEntry drag_targets[] = {
                { "text/uri-list", 0, 0 }, { "application/x-desktop", 0, 0 },
        };

        /* Main layout */
        layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_container_add(GTK_CONTAINER(self), layout);

        /* Image on the left */
        image = gtk_image_new();
        self->image = image;
        gtk_widget_set_margin_end(image, 7);
        gtk_box_pack_start(GTK_BOX(layout), image, FALSE, FALSE, 0);

        /* Display label */
        label = gtk_label_new("");
        self->label = label;
        g_object_set(self->label, "halign", GTK_ALIGN_START, "valign", GTK_ALIGN_CENTER, NULL);
        gtk_box_pack_start(GTK_BOX(layout), label, TRUE, TRUE, 0);

        /* Button specific fixes */
        gtk_button_set_relief(GTK_BUTTON(self), GTK_RELIEF_NONE);
        gtk_widget_set_can_focus(GTK_WIDGET(self), FALSE);

        /* Flatten the button */
        style = gtk_widget_get_style_context(GTK_WIDGET(self));
        gtk_style_context_add_class(style, "flat");

        /* Hook up drag so users can drag .desktop from here elsewhere */
        gtk_drag_source_set(GTK_WIDGET(self), GDK_BUTTON1_MASK, drag_targets, 2, GDK_ACTION_COPY);
}

/**
 * Handle setting the icon according to our desktop file
 */
static void brisk_menu_entry_drag_begin(GtkWidget *widget, GdkDragContext *context)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(widget);

        GIcon *icon = NULL;

        /* If we have a .desktop & icon, use it */
        if (self->info) {
                icon = g_app_info_get_icon(G_APP_INFO(self->info));
                gtk_drag_set_icon_gicon(context, icon, 0, 0);
                return;
        }

        /* Fallback to the default */
        gtk_drag_set_icon_default(context);
}

static void brisk_menu_entry_drag_data(GtkWidget *widget, __brisk_unused__ GdkDragContext *context,
                                       GtkSelectionData *data, __brisk_unused__ guint info,
                                       __brisk_unused__ guint time)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(widget);

        if (!self->info) {
                return;
        }
        const gchar *uris[2];

        const gchar *desktop_path = NULL;
        autofree(gchar) *uri = NULL;

        desktop_path = g_desktop_app_info_get_filename(self->info);
        uri = g_filename_to_uri(desktop_path, NULL, NULL);
        if (!uri) {
                return;
        }
        uris[0] = uri;
        uris[1] = NULL;

        gtk_selection_data_set_uris(data, (gchar **)uris);
}

static void brisk_menu_entry_clicked(GtkButton *widget)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(widget);

        if (!self->info) {
                return;
        }
        brisk_menu_launcher_start(self->launcher, GTK_WIDGET(self), G_APP_INFO(self->info));
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
