/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2016-2017 Ikey Doherty <ikey@solus-project.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "backend/item.h"
#include "entry-button.h"
#include "launcher.h"
#include "menu-private.h"
#include <gtk/gtk.h>

BRISK_END_PEDANTIC

struct _BriskMenuEntryButtonClass {
        GtkButtonClass parent_class;
};

static void brisk_menu_entry_drag_begin(GtkWidget *widget, GdkDragContext *context);
static void brisk_menu_entry_drag_end(GtkWidget *widget, GdkDragContext *context);
static void brisk_menu_entry_drag_data(GtkWidget *widget, GdkDragContext *context,
                                       GtkSelectionData *data, guint info, guint time);
static void brisk_menu_entry_clicked(GtkButton *widget);

/**
 * BriskMenuEntryButton is the toplevel window type used within the applet.
 */
struct _BriskMenuEntryButton {
        GtkButton parent;
        GtkWidget *label;
        GtkWidget *image;
        BriskItem *item;
        BriskMenuLauncher *launcher;
};

G_DEFINE_TYPE(BriskMenuEntryButton, brisk_menu_entry_button, GTK_TYPE_BUTTON)

enum { PROP_ITEM = 1, PROP_LAUNCHER, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void brisk_menu_entry_button_set_property(GObject *object, guint id, const GValue *value,
                                                 GParamSpec *spec)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(object);

        switch (id) {
        case PROP_ITEM:
                self->item = g_value_get_object(value);
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
        case PROP_ITEM:
                g_value_set_object(value, self->item);
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
GtkWidget *brisk_menu_entry_button_new(BriskMenuLauncher *launcher, BriskItem *item)
{
        return g_object_new(BRISK_TYPE_MENU_ENTRY_BUTTON, "launcher", launcher, "item", item, NULL);
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
        g_clear_object(&self->item);

        G_OBJECT_CLASS(brisk_menu_entry_button_parent_class)->dispose(obj);
}

/**
 * Handle constructor specifics for our button
 */
static void brisk_menu_entry_button_constructed(GObject *obj)
{
        BriskMenuEntryButton *self = NULL;
        const GIcon *icon = NULL;

        self = BRISK_MENU_ENTRY_BUTTON(obj);

        icon = brisk_item_get_icon(self->item);
        if (icon) {
                gtk_image_set_from_gicon(GTK_IMAGE(self->image),
                                         (GIcon *)icon,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);
        } else {
                gtk_image_set_from_icon_name(GTK_IMAGE(self->image),
                                             "image-missing",
                                             GTK_ICON_SIZE_LARGE_TOOLBAR);
        }

        gtk_image_set_pixel_size(GTK_IMAGE(self->image), 24);

        /* Determine our label based on the app */
        gtk_label_set_label(GTK_LABEL(self->label), brisk_item_get_name(self->item));
        gtk_widget_set_tooltip_text(GTK_WIDGET(self), brisk_item_get_summary(self->item));

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
        wid_class->drag_end = brisk_menu_entry_drag_end;

        /* button vtable hookup */
        but_class->clicked = brisk_menu_entry_clicked;

        obj_properties[PROP_ITEM] = g_param_spec_object("item",
                                                        "The BriskItem",
                                                        "Corresponding BriskItem",
                                                        BRISK_TYPE_ITEM,
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
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_FLAT);

        /* Hook up drag so users can drag .desktop from here elsewhere */
        gtk_drag_source_set(GTK_WIDGET(self), GDK_BUTTON1_MASK, drag_targets, 2, GDK_ACTION_COPY);
}

/**
 * Handle setting the icon according to our desktop file
 * We ref the icon here so we can immediately hide the toplevel to get it out
 * of the way.
 */
static void brisk_menu_entry_drag_begin(GtkWidget *widget, GdkDragContext *context)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(widget);
        const GIcon *icon = NULL;
        GIcon *reffed_icon = NULL;

        /* If we have a .desktop icon, use it */
        icon = brisk_item_get_icon(self->item);

        /* Fallback to the default */
        if (!icon) {
                gtk_drag_set_icon_default(context);
                return;
        }

        reffed_icon = g_object_ref((GIcon *)icon);
        g_object_set_data_full(G_OBJECT(context), "_drag_icon_brisk", reffed_icon, g_object_unref);
        gtk_drag_set_icon_gicon(context, reffed_icon, 0, 0);
}

static gboolean hide_toplevel(GtkWidget *widget)
{
        GtkWidget *toplevel = NULL;

        /* Hide toplevel to avoid grab problems */
        toplevel = gtk_widget_get_toplevel(widget);
        gtk_widget_hide(toplevel);

        return FALSE;
}

/**
 * Clean up the ref'd icon
 */
static void brisk_menu_entry_drag_end(__brisk_unused__ GtkWidget *widget, GdkDragContext *context)
{
        GIcon *icon = NULL;

        g_idle_add((GSourceFunc)hide_toplevel, widget);

        icon = g_object_get_data(G_OBJECT(context), "_drag_icon_brisk");
        if (!icon) {
                return;
        }
        g_object_set_data(G_OBJECT(context), "_drag_icon_brisk", NULL);
}

static void brisk_menu_entry_drag_data(GtkWidget *widget, __brisk_unused__ GdkDragContext *context,
                                       GtkSelectionData *data, __brisk_unused__ guint info,
                                       __brisk_unused__ guint time)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(widget);

        const gchar *uris[2];

        const gchar *desktop_path = NULL;
        autofree(gchar) *uri = NULL;

        /* TODO: ADD ITEM API !
        desktop_path = g_desktop_app_info_get_filename(self->info);
        uri = g_filename_to_uri(desktop_path, NULL, NULL); */
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
        brisk_menu_entry_button_launch(self);
}

void brisk_menu_entry_button_launch(BriskMenuEntryButton *self)
{
        /* TODO: Expose launcher to the item */
        brisk_item_launch(self->item);
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
