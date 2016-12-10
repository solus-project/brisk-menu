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
#include "category-button.h"
#include <gtk/gtk.h>
SOLUS_END_PEDANTIC

struct _SolMenuCategoryButtonClass {
        GtkWindowClass parent_class;
};

/**
 * SolMenuCategoryButton is the toplevel window type used within the applet.
 */
struct _SolMenuCategoryButton {
        int __reserved1;
};

G_DEFINE_TYPE(SolMenuCategoryButton, sol_menu_category_button, GTK_TYPE_RADIO_BUTTON)

/**
 * sol_menu_category_button_new:
 *
 * Construct a new SolMenuCategoryButton object
 */
GtkWidget *sol_menu_category_button_new()
{
        return g_object_new(SOL_TYPE_MENU_CATEGORY_BUTTON, NULL);
}

/**
 * sol_menu_category_button_dispose:
 *
 * Clean up a SolMenuCategoryButton instance
 */
static void sol_menu_category_button_dispose(GObject *obj)
{
        G_OBJECT_CLASS(sol_menu_category_button_parent_class)->dispose(obj);
}

/**
 * sol_menu_category_button_class_init:
 *
 * Handle class initialisation
 */
static void sol_menu_category_button_class_init(SolMenuCategoryButtonClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = sol_menu_category_button_dispose;
}

/**
 * sol_menu_category_button_init:
 *
 * Handle construction of the SolMenuCategoryButton
 */
static void sol_menu_category_button_init(__solus_unused__ SolMenuCategoryButton *self)
{
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
