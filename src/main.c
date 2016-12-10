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

#include "util.h"

#include <stdlib.h>

SOLUS_BEGIN_PEDANTIC
#include "menu-window.h"
#include <gtk/gtk.h>
SOLUS_END_PEDANTIC

int main(int argc, char **argv)
{
        gtk_init(&argc, &argv);
        GtkWidget *menu_window = NULL;

        menu_window = sol_menu_window_new();
        gtk_widget_show_all(menu_window);
        g_signal_connect(menu_window, "destroy", gtk_main_quit, NULL);
        gtk_main();
        gtk_widget_destroy(menu_window);
        menu_window = NULL;

        return EXIT_SUCCESS;
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
