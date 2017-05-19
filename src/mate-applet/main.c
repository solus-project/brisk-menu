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

#include "config.h"
#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "applet.h"
#include "brisk-resources.h"
#include <glib/gi18n.h>
#include <mate-panel-applet.h>
BRISK_END_PEDANTIC

DEF_AUTOFREE(GtkActionGroup, g_object_unref)

/**
 * We have no .ctor in the .a file - so it doesn't link
 */
__attribute__((constructor)) static void brisk_resource_init(void)
{
        brisk_resources_register_resource();
}

/**
 * Again, no .dtor due to link issues, so we do it here
 */
__attribute__((destructor)) static void brisk_resource_deinit(void)
{
        brisk_resources_unregister_resource();
}

/**
 * Menu actions for right click on the button
 */
G_GNUC_BEGIN_IGNORE_DEPRECATIONS
static const GtkActionEntry brisk_actions[] = { {
    "EditMenus",
    GTK_STOCK_EDIT,
    N_("_Edit Menus"),
    NULL,
    NULL,
    G_CALLBACK(brisk_menu_applet_edit_menus),
} };
G_GNUC_END_IGNORE_DEPRECATIONS

/**
 * UI definition for our right click menu
 */
#define BRISK_MENU_XML "<menuitem name=\"Edit Menus\" action=\"EditMenus\" />"

static gboolean brisk_menu_applet_factory(MatePanelApplet *applet, const gchar *id,
                                          __brisk_unused__ gpointer v)
{
        if (!g_str_has_prefix(id, "BriskMenu")) {
                return FALSE;
        }
        const char *home = NULL;
        __attribute__((unused)) int ret = 0;
        autofree(GtkActionGroup) *group = NULL;

        home = g_get_home_dir();
        if (home) {
                ret = chdir(home);
        }

        /* Setup the action group and hand it to the mate panel */
        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        group = gtk_action_group_new("Brisk Menu Actions");
        gtk_action_group_set_translation_domain(group, GETTEXT_PACKAGE);
        gtk_action_group_add_actions(group, brisk_actions, G_N_ELEMENTS(brisk_actions), applet);
        mate_panel_applet_setup_menu(applet, BRISK_MENU_XML, group);
        G_GNUC_END_IGNORE_DEPRECATIONS

        g_set_application_name(_("Brisk Menu Launcher"));
        gtk_widget_show(GTK_WIDGET(applet));
        return TRUE;
}

MATE_PANEL_APPLET_OUT_PROCESS_FACTORY("BriskMenuFactory", BRISK_TYPE_MENU_APPLET, "BriskMenu",
                                      brisk_menu_applet_factory, NULL)

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
