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

#define _GNU_SOURCE

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "category-button.h"
#include "menu-private.h"
#include <glib/gi18n.h>
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

struct _BriskDashCategoryButtonClass {
        GtkRadioButtonClass parent_class;
};

/**
 * BriskDashCategoryButton is the toplevel window type used within the applet.
 */
struct _BriskDashCategoryButton {
        GtkRadioButton parent;
        BriskSection *section;
        GtkWidget *label;
};

G_DEFINE_TYPE(BriskDashCategoryButton, brisk_dash_category_button, GTK_TYPE_RADIO_BUTTON)

enum { PROP_SECTION = 1, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void brisk_dash_category_button_set_property(GObject *object, guint id, const GValue *value,
                                                    GParamSpec *spec)
{
        BriskDashCategoryButton *self = BRISK_DASH_CATEGORY_BUTTON(object);

        switch (id) {
        case PROP_SECTION:
                self->section = g_value_get_pointer(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void brisk_dash_category_button_get_property(GObject *object, guint id, GValue *value,
                                                    GParamSpec *spec)
{
        BriskDashCategoryButton *self = BRISK_DASH_CATEGORY_BUTTON(object);

        switch (id) {
        case PROP_SECTION:
                g_value_set_pointer(value, self->section);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * brisk_dash_category_button_new:
 *
 * Construct a new BriskDashCategoryButton object
 */
GtkWidget *brisk_dash_category_button_new(BriskSection *section)
{
        return g_object_new(BRISK_TYPE_DASH_CATEGORY_BUTTON, "section", section, NULL);
}

/**
 * brisk_dash_category_button_dispose:
 *
 * Clean up a BriskDashCategoryButton instance
 */
static void brisk_dash_category_button_dispose(GObject *obj)
{
        BriskDashCategoryButton *self = BRISK_DASH_CATEGORY_BUTTON(obj);

        g_clear_object(&self->section);

        G_OBJECT_CLASS(brisk_dash_category_button_parent_class)->dispose(obj);
}

/**
 * Handle constructor specifics for our button
 */
static void brisk_dash_category_button_constructed(GObject *obj)
{
        BriskDashCategoryButton *self = NULL;

        self = BRISK_DASH_CATEGORY_BUTTON(obj);

        /* If we have a section, use it, otherwise we're a special "All" button */
        if (self->section) {
                gtk_label_set_label(GTK_LABEL(self->label), brisk_section_get_name(self->section));
        } else {
                gtk_label_set_label(GTK_LABEL(self->label), _("All"));
        }

        G_OBJECT_CLASS(brisk_dash_category_button_parent_class)->constructed(obj);
}

/**
 * brisk_dash_category_button_class_init:
 *
 * Handle class initialisation
 */
static void brisk_dash_category_button_class_init(BriskDashCategoryButtonClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_dash_category_button_dispose;
        obj_class->set_property = brisk_dash_category_button_set_property;
        obj_class->get_property = brisk_dash_category_button_get_property;
        obj_class->constructed = brisk_dash_category_button_constructed;

        obj_properties[PROP_SECTION] = g_param_spec_pointer("section",
                                                            "The BriskSection",
                                                            "Section that this category represents",
                                                            G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

/**
 * brisk_dash_category_button_init:
 *
 * Handle construction of the BriskDashCategoryButton
 */
static void brisk_dash_category_button_init(BriskDashCategoryButton *self)
{
        GtkStyleContext *style = NULL;
        GtkWidget *label = NULL;
        GtkWidget *layout = NULL;

        /* Main layout */
        layout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_container_add(GTK_CONTAINER(self), layout);

        /* Display label */
        label = gtk_label_new("");
        self->label = label;
        g_object_set(self->label, "halign", GTK_ALIGN_START, "valign", GTK_ALIGN_CENTER, NULL);
        gtk_box_pack_start(GTK_BOX(layout), label, TRUE, TRUE, 0);

        /* Button specific fixes */
        gtk_widget_set_can_focus(GTK_WIDGET(self), FALSE);

        /* Look like a button */
        g_object_set(G_OBJECT(self), "draw-indicator", FALSE, NULL);
        gtk_widget_set_can_focus(GTK_WIDGET(self), FALSE);

        /* Flatten the button */
        style = gtk_widget_get_style_context(GTK_WIDGET(self));
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_FLAT);
        gtk_style_context_add_class(style, "dash-category-button");
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
