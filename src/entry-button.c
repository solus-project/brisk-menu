/*
 * This file is part of mate-solmenu.
 *
 * Copyright © 2016 Ikey Doherty <ikey@solus-proejct.com>
 *
 * mate-solmenu is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include "util.h"

SOLUS_BEGIN_PEDANTIC
#include "entry-button.h"
#include "menu-private.h"
#include <gtk/gtk.h>
#include <matemenu-tree.h>
SOLUS_END_PEDANTIC

struct _SolMenuEntryButtonClass {
        GtkButtonClass parent_class;
};

/**
 * SolMenuEntryButton is the toplevel window type used within the applet.
 */
struct _SolMenuEntryButton {
        GtkButton parent;
        MateMenuTreeEntry *entry;
        GtkWidget *label;
        GtkWidget *image;
};

G_DEFINE_TYPE(SolMenuEntryButton, sol_menu_entry_button, GTK_TYPE_BUTTON)

enum { PROP_ENTRY = 1, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void sol_menu_entry_button_set_property(GObject *object, guint id, const GValue *value,
                                               GParamSpec *spec)
{
        SolMenuEntryButton *self = SOL_MENU_ENTRY_BUTTON(object);

        switch (id) {
        case PROP_ENTRY:
                self->entry = g_value_get_pointer(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void sol_menu_entry_button_get_property(GObject *object, guint id, GValue *value,
                                               GParamSpec *spec)
{
        SolMenuEntryButton *self = SOL_MENU_ENTRY_BUTTON(object);

        switch (id) {
        case PROP_ENTRY:
                g_value_set_pointer(value, self->entry);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * sol_menu_entry_button_new:
 *
 * Construct a new SolMenuEntryButton object
 */
GtkWidget *sol_menu_entry_button_new(MateMenuTreeEntry *entry)
{
        return g_object_new(SOL_TYPE_MENU_ENTRY_BUTTON, "entry", entry, NULL);
}

/**
 * sol_menu_entry_button_dispose:
 *
 * Clean up a SolMenuEntryButton instance
 */
static void sol_menu_entry_button_dispose(GObject *obj)
{
        G_OBJECT_CLASS(sol_menu_entry_button_parent_class)->dispose(obj);
}

static GIcon *sol_menu_entry_button_create_gicon(const char *path)
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
static void sol_menu_entry_button_constructed(GObject *obj)
{
        const gchar *label = NULL;
        SolMenuEntryButton *self = NULL;
        const gchar *icon_name = NULL;

        self = SOL_MENU_ENTRY_BUTTON(obj);

        /* matemenu has no gicon support, so do it ourselves. */
        icon_name = matemenu_tree_entry_get_icon(self->entry);
        if (icon_name && icon_name[0] == '/') {
                autofree(GIcon) *ico = sol_menu_entry_button_create_gicon(icon_name);
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

        G_OBJECT_CLASS(sol_menu_entry_button_parent_class)->constructed(obj);
}

/**
 * sol_menu_entry_button_class_init:
 *
 * Handle class initialisation
 */
static void sol_menu_entry_button_class_init(SolMenuEntryButtonClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = sol_menu_entry_button_dispose;
        obj_class->set_property = sol_menu_entry_button_set_property;
        obj_class->get_property = sol_menu_entry_button_get_property;
        obj_class->constructed = sol_menu_entry_button_constructed;

        obj_properties[PROP_ENTRY] = g_param_spec_pointer("entry",
                                                          "The MateMenuTreeEntry",
                                                          "Entry that this launcher represents",
                                                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);
        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

/**
 * sol_menu_entry_button_init:
 *
 * Handle construction of the SolMenuEntryButton
 */
static void sol_menu_entry_button_init(SolMenuEntryButton *self)
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
