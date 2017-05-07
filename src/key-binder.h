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

#pragma once

#include <gdk/gdk.h>
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * BriskKeyBinder is used to bind global x11 shortcuts
 */
struct _BriskKeyBinder {
        GObject parent;
        GdkWindow *root_window;
        GHashTable *bindings;
        const gchar *shortcut;
        gboolean wait_for_release;
};

typedef struct _BriskKeyBinder BriskKeyBinder;
typedef struct _BriskKeyBinderClass BriskKeyBinderClass;

#define BRISK_TYPE_KEY_BINDER brisk_key_binder_get_type()
#define BRISK_KEY_BINDER(o) (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_KEY_BINDER, BriskKeyBinder))
#define BRISK_IS_KEY_BINDER(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_KEY_BINDER))
#define BRISK_KEY_BINDER_CLASS(o)                                                                  \
        (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_KEY_BINDER, BriskKeyBinderClass))
#define BRISK_IS_KEY_BINDER_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_KEY_BINDER))
#define BRISK_KEY_BINDER_GET_CLASS(o)                                                              \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_KEY_BINDER, BriskKeyBinderClass))

/**
 * Construct a new BriskKeyBinder
 */
BriskKeyBinder *brisk_key_binder_new(void);

GType brisk_key_binder_get_type(void);

/**
 * A callback for the binder functions
 */
typedef void (*BinderFunc)(GdkEvent *event, gpointer v);

gboolean brisk_key_binder_bind(BriskKeyBinder *self, const gchar *shortcut, BinderFunc func,
                               gpointer v);
gboolean brisk_key_binder_unbind(BriskKeyBinder *self, const gchar *shortcut);

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
