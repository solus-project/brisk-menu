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

#pragma once

#include "menu-window.h"
#include <gtk/gtk.h>

struct _SolMenuWindowClass {
        GtkWindowClass parent_class;
};

/**
 * SolMenuWindow is the toplevel window type used within the applet.
 */
struct _SolMenuWindow {
        GtkWindow parent;

        /* Categories */
        GtkWidget *sidebar;

        /* Top search entry */
        GtkWidget *search;

        /* Actual applications */
        GtkWidget *apps;
};

void sol_menu_window_load_menus(SolMenuWindow *self);

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
