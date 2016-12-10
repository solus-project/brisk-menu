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

#include <glib-object.h>
#include <gtk/gtk.h>
#include <matemenu-tree.h>

G_BEGIN_DECLS

typedef struct _SolMenuEntryButton SolMenuEntryButton;
typedef struct _SolMenuEntryButtonClass SolMenuEntryButtonClass;

#define SOL_TYPE_MENU_ENTRY_BUTTON sol_menu_entry_button_get_type()
#define SOL_MENU_ENTRY_BUTTON(o)                                                                   \
        (G_TYPE_CHECK_INSTANCE_CAST((o), SOL_TYPE_MENU_ENTRY_BUTTON, SolMenuEntryButton))
#define SOL_IS_MENU_ENTRY_BUTTON(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SOL_TYPE_MENU_ENTRY_BUTTON))
#define SOL_MENU_ENTRY_BUTTON_CLASS(o)                                                             \
        (G_TYPE_CHECK_CLASS_CAST((o), SOL_TYPE_MENU_ENTRY_BUTTON, SolMenuEntryButtonClass))
#define SOL_IS_MENU_ENTRY_BUTTON_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), SOL_TYPE_MENU_ENTRY_BUTTON))
#define SOL_MENU_ENTRY_BUTTON_GET_CLASS(o)                                                         \
        (G_TYPE_INSTANCE_GET_CLASS((o), SOL_TYPE_MENU_ENTRY_BUTTON, SolMenuEntryButtonClass))

/**
 * Construct a new SolMenuEntryButton from the given entry
 */
GtkWidget *sol_menu_entry_button_new(MateMenuTreeEntry *entry);

GType sol_menu_entry_button_get_type(void);

G_END_DECLS

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
