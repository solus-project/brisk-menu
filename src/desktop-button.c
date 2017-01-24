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
#include "desktop-button.h"
#include "menu-private.h"
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

struct _BriskMenuDesktopButtonClass {
        GtkButtonClass parent_class;
};

/**
 * BriskMenuDesktopButton is the toplevel window type used within the applet.
 */
struct _BriskMenuDesktopButton {
        GtkButton parent;
        GAppInfo *desktop;
        GtkWidget *label;
        GtkWidget *image;
        BriskMenuLauncher *launcher;
};

G_DEFINE_TYPE(BriskMenuDesktopButton, brisk_menu_desktop_button, GTK_TYPE_BUTTON)

static void brisk_menu_desktop_button_clicked(GtkButton *button);

enum { PROP_DESKTOP = 1, PROP_LAUNCHER, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void brisk_menu_desktop_button_set_property(GObject *object, guint id, const GValue *value,
                                                   GParamSpec *spec)
{
        BriskMenuDesktopButton *self = BRISK_MENU_DESKTOP_BUTTON(object);

        switch (id) {
        case PROP_DESKTOP:
                self->desktop = g_value_get_pointer(value);
                break;
        case PROP_LAUNCHER:
                self->launcher = g_value_get_pointer(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void brisk_menu_desktop_button_get_property(GObject *object, guint id, GValue *value,
                                                   GParamSpec *spec)
{
        BriskMenuDesktopButton *self = BRISK_MENU_DESKTOP_BUTTON(object);

        switch (id) {
        case PROP_DESKTOP:
                g_value_set_pointer(value, self->desktop);
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
 * brisk_menu_desktop_button_new:
 *
 * Construct a new BriskMenuDesktopButton object
 */
GtkWidget *brisk_menu_desktop_button_new(BriskMenuLauncher *launcher, GAppInfo *desktop)
{
        return g_object_new(BRISK_TYPE_MENU_DESKTOP_BUTTON,
                            "launcher",
                            launcher,
                            "desktop",
                            desktop,
                            NULL);
}

/**
 * brisk_menu_desktop_button_dispose:
 *
 * Clean up a BriskMenuDesktopButton instance
 */
static void brisk_menu_desktop_button_dispose(GObject *obj)
{
        BriskMenuDesktopButton *self = NULL;

        self = BRISK_MENU_DESKTOP_BUTTON(obj);
        g_clear_object(&self->desktop);

        G_OBJECT_CLASS(brisk_menu_desktop_button_parent_class)->dispose(obj);
}

/**
 * Handle constructor specifics for our button
 */
static void brisk_menu_desktop_button_constructed(GObject *obj)
{
        const gchar *label = NULL;
        BriskMenuDesktopButton *self = NULL;

        self = BRISK_MENU_DESKTOP_BUTTON(obj);

        gtk_image_set_from_gicon(GTK_IMAGE(self->image),
                                 g_app_info_get_icon(self->desktop),
                                 GTK_ICON_SIZE_BUTTON);
        gtk_image_set_pixel_size(GTK_IMAGE(self->image), 16);

        /* Determine our label based on the app */
        label = g_app_info_get_display_name(self->desktop);
        gtk_label_set_label(GTK_LABEL(self->label), label);
        gtk_widget_set_tooltip_text(GTK_WIDGET(self), g_app_info_get_description(self->desktop));

        G_OBJECT_CLASS(brisk_menu_desktop_button_parent_class)->constructed(obj);
}

/**
 * brisk_menu_desktop_button_class_init:
 *
 * Handle class initialisation
 */
static void brisk_menu_desktop_button_class_init(BriskMenuDesktopButtonClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        GtkButtonClass *but_class = GTK_BUTTON_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_menu_desktop_button_dispose;
        obj_class->set_property = brisk_menu_desktop_button_set_property;
        obj_class->get_property = brisk_menu_desktop_button_get_property;
        obj_class->constructed = brisk_menu_desktop_button_constructed;

        /* button vtable hookup */
        but_class->clicked = brisk_menu_desktop_button_clicked;

        obj_properties[PROP_DESKTOP] = g_param_spec_pointer("desktop",
                                                            "The GAppInfo",
                                                            "Desktop file",
                                                            G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        obj_properties[PROP_LAUNCHER] = g_param_spec_pointer("launcher",
                                                             "The Brisk Launcher",
                                                             "Launcher used for starting apps",
                                                             G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

/**
 * brisk_menu_desktop_button_init:
 *
 * Handle construction of the BriskMenuDesktopButton
 */
static void brisk_menu_desktop_button_init(BriskMenuDesktopButton *self)
{
        GtkStyleContext *style = NULL;
        GtkWidget *label = NULL;
        GtkWidget *image = NULL;
        GtkWidget *layout = NULL;

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
        gtk_widget_set_can_focus(GTK_WIDGET(self), FALSE);
        gtk_button_set_relief(GTK_BUTTON(self), GTK_RELIEF_NONE);

        /* Flatten the button */
        style = gtk_widget_get_style_context(GTK_WIDGET(self));
        gtk_style_context_add_class(style, "flat");
}

static void brisk_menu_desktop_button_clicked(GtkButton *button)
{
        BriskMenuDesktopButton *self = NULL;

        self = BRISK_MENU_DESKTOP_BUTTON(button);
        brisk_menu_launcher_start(self->launcher, GTK_WIDGET(self), self->desktop);
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
