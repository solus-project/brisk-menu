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

#pragma once

#include "backend/section.h"
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BriskClassicCategoryButton BriskClassicCategoryButton;
typedef struct _BriskClassicCategoryButtonClass BriskClassicCategoryButtonClass;

#define BRISK_TYPE_CLASSIC_CATEGORY_BUTTON brisk_classic_category_button_get_type()
#define BRISK_CLASSIC_CATEGORY_BUTTON(o)                                                           \
        (G_TYPE_CHECK_INSTANCE_CAST((o),                                                           \
                                    BRISK_TYPE_CLASSIC_CATEGORY_BUTTON,                            \
                                    BriskClassicCategoryButton))
#define BRISK_IS_CLASSIC_CATEGORY_BUTTON(o)                                                        \
        (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_CLASSIC_CATEGORY_BUTTON))
#define BRISK_CLASSIC_CATEGORY_BUTTON_CLASS(o)                                                     \
        (G_TYPE_CHECK_CLASS_CAST((o),                                                              \
                                 BRISK_TYPE_CLASSIC_CATEGORY_BUTTON,                               \
                                 BriskClassicCategoryButtonClass))
#define BRISK_IS_CLASSIC_CATEGORY_BUTTON_CLASS(o)                                                  \
        (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_CLASSIC_CATEGORY_BUTTON))
#define BRISK_CLASSIC_CATEGORY_BUTTON_GET_CLASS(o)                                                 \
        (G_TYPE_INSTANCE_GET_CLASS((o),                                                            \
                                   BRISK_TYPE_CLASSIC_CATEGORY_BUTTON,                             \
                                   BriskClassicCategoryButtonClass))

/**
 * Construct a new BriskClassicCategoryButton from the given group
 */
GtkWidget *brisk_classic_category_button_new(BriskSection *section);

GType brisk_classic_category_button_get_type(void);

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
