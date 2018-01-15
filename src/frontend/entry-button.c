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
#include "backend/item.h"
#include "entry-button.h"
#include "launcher.h"
#include "menu-private.h"
#include <gtk/gtk.h>
BRISK_END_PEDANTIC

static void brisk_menu_entry_drag_begin(GtkWidget *widget, GdkDragContext *context);
static void brisk_menu_entry_drag_end(GtkWidget *widget, GdkDragContext *context);
static void brisk_menu_entry_drag_data(GtkWidget *widget, GdkDragContext *context,
                                       GtkSelectionData *data, guint info, guint time);
static gboolean brisk_menu_entry_button_release_event(GtkWidget *wid, GdkEventButton *event);

/**
 * IDs for our signals
 */
enum { ENTRY_BUTTON_SIGNAL_CONTEXT_MENU = 0, N_SIGNALS };

static guint entry_button_signals[N_SIGNALS] = { 0 };

G_DEFINE_TYPE(BriskMenuEntryButton, brisk_menu_entry_button, GTK_TYPE_BUTTON)

enum { PROP_ITEM = 1, PROP_LAUNCHER, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

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

static void brisk_menu_entry_button_set_property(GObject *object, guint id, const GValue *value,
                                                 GParamSpec *spec)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(object);

        switch (id) {
        case PROP_ITEM:
                self->item = g_value_get_pointer(value);
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
                g_value_set_pointer(value, self->item);
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
 * brisk_menu_entry_button_class_init:
 *
 * Handle class initialisation
 */
static void brisk_menu_entry_button_class_init(BriskMenuEntryButtonClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        GtkWidgetClass *wid_class = GTK_WIDGET_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = brisk_menu_entry_button_dispose;
        obj_class->set_property = brisk_menu_entry_button_set_property;
        obj_class->get_property = brisk_menu_entry_button_get_property;

        /* widget vtable hookup */
        wid_class->drag_data_get = brisk_menu_entry_drag_data;
        wid_class->drag_begin = brisk_menu_entry_drag_begin;
        wid_class->drag_end = brisk_menu_entry_drag_end;
        wid_class->button_release_event = brisk_menu_entry_button_release_event;

        /**
         * BriskEntryButton::show-context-menu
         * @button: The button that created the ievent
         * @item: The menu to show a menu for
         *
         * Used to notify the frontend that we need an action menu displayed
         */
        entry_button_signals[ENTRY_BUTTON_SIGNAL_CONTEXT_MENU] =
            g_signal_new("show-context-menu",
                         BRISK_TYPE_MENU_ENTRY_BUTTON,
                         G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                         G_STRUCT_OFFSET(BriskMenuEntryButtonClass, show_context_menu),
                         NULL,
                         NULL,
                         NULL,
                         G_TYPE_NONE,
                         1,
                         BRISK_TYPE_ITEM);

        obj_properties[PROP_ITEM] = g_param_spec_pointer("item",
                                                         "The BriskItem",
                                                         "Corresponding BriskItem",
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
        static const GtkTargetEntry drag_targets[] = {
                { "text/uri-list", 0, 0 },
                { "application/x-desktop", 0, 0 },
        };

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
        autofree(gchar) *uri = NULL;

        uri = brisk_item_get_uri(self->item);
        if (!uri) {
                return;
        }
        uris[0] = uri;
        uris[1] = NULL;

        gtk_selection_data_set_uris(data, (gchar **)uris);
}

static gboolean brisk_menu_entry_button_release_event(GtkWidget *widget,
                                                      GdkEventButton *event_button)
{
        BriskMenuEntryButton *self = BRISK_MENU_ENTRY_BUTTON(widget);

        switch (event_button->button) {
        case GDK_BUTTON_PRIMARY:
                /* Left click, primary launch action */
                brisk_menu_entry_button_launch(self);
                break;
        case GDK_BUTTON_SECONDARY:
                /* Right click, ask for a context menu */
                g_signal_emit(self,
                              entry_button_signals[ENTRY_BUTTON_SIGNAL_CONTEXT_MENU],
                              0,
                              self->item);
                break;
        default:
                break;
        }

        return GTK_WIDGET_CLASS(brisk_menu_entry_button_parent_class)
            ->button_press_event(widget, event_button);
}

void brisk_menu_entry_button_launch(BriskMenuEntryButton *self)
{
        brisk_menu_launcher_start_item(self->launcher, GTK_WIDGET(self), self->item);
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
