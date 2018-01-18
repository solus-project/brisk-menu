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

#include "styles.h"
#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "dash-entry-button.h"
#include <glib/gi18n.h>
BRISK_END_PEDANTIC

G_DEFINE_TYPE(BriskDashEntryButton, brisk_dash_entry_button, BRISK_TYPE_MENU_ENTRY_BUTTON)

/**
 * Handle constructor specifics for our button
 */
static void brisk_dash_entry_button_constructed(GObject *obj)
{
        BriskDashEntryButton *self = NULL;
        const GIcon *icon = NULL;

        self = BRISK_DASH_ENTRY_BUTTON(obj);

        icon = brisk_item_get_icon(BRISK_MENU_ENTRY_BUTTON(self)->item);
        if (icon) {
                gtk_image_set_from_gicon(GTK_IMAGE(self->image),
                                         (GIcon *)icon,
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);
        } else {
                gtk_image_set_from_icon_name(GTK_IMAGE(self->image),
                                             "image-missing",
                                             GTK_ICON_SIZE_LARGE_TOOLBAR);
        }

        gtk_image_set_pixel_size(GTK_IMAGE(self->image), 64);

        /* Determine our label based on the app */
        gtk_label_set_label(GTK_LABEL(self->label),
                            brisk_item_get_name(BRISK_MENU_ENTRY_BUTTON(self)->item));
        gtk_widget_set_tooltip_text(GTK_WIDGET(self),
                                    brisk_item_get_summary(BRISK_MENU_ENTRY_BUTTON(self)->item));

        G_OBJECT_CLASS(brisk_dash_entry_button_parent_class)->constructed(obj);
}

static void brisk_dash_entry_button_dispose(GObject *obj)
{
        G_OBJECT_CLASS(brisk_dash_entry_button_parent_class)->dispose(obj);
}

/**
 * brisk_dash_entry_button_class_init:
 *
 * Handle class initialisation
 */
static void brisk_dash_entry_button_class_init(BriskDashEntryButtonClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->constructed = brisk_dash_entry_button_constructed;
        obj_class->dispose = brisk_dash_entry_button_dispose;
}

/**
 * brisk_dash_entry_button_init:
 *
 * Handle construction of the BriskDashEntryButton
 */
static void brisk_dash_entry_button_init(BriskDashEntryButton *self)
{
        GtkStyleContext *style = NULL;
        GtkWidget *label = NULL;
        GtkWidget *image = NULL;
        GtkWidget *layout = NULL;

        /* Main layout */
        layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_add(GTK_CONTAINER(self), layout);

        /* Image on the left */
        image = gtk_image_new();
        self->image = image;
        gtk_widget_set_margin_bottom(image, 7);
        gtk_box_pack_start(GTK_BOX(layout), image, FALSE, FALSE, 0);

        /* Display label */
        label = gtk_label_new("");
        self->label = label;
        gtk_label_set_lines(GTK_LABEL(label), 2);
        gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
        gtk_label_set_max_width_chars(GTK_LABEL(label), 15);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        g_object_set(self->label,
                     "halign",
                     GTK_ALIGN_CENTER,
                     "valign",
                     GTK_ALIGN_START,
                     "justify",
                     GTK_JUSTIFY_CENTER,
                     NULL);
        gtk_box_pack_start(GTK_BOX(layout), label, TRUE, TRUE, 0);

        /* Button specific fixes */
        gtk_button_set_relief(GTK_BUTTON(self), GTK_RELIEF_NONE);
        gtk_widget_set_can_focus(GTK_WIDGET(self), FALSE);

        /* Flatten the button */
        style = gtk_widget_get_style_context(GTK_WIDGET(self));
        gtk_style_context_add_class(style, GTK_STYLE_CLASS_FLAT);

        gtk_widget_show_all(layout);
}

/**
 * brisk_dash_entry_button_new:
 *
 * Return a newly created BriskDashEntryButton
 */
BriskMenuEntryButton *brisk_dash_entry_button_new(BriskMenuLauncher *launcher, BriskItem *item)
{
        return g_object_new(BRISK_TYPE_DASH_ENTRY_BUTTON, "launcher", launcher, "item", item, NULL);
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
