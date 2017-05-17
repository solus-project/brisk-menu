/*
 * This file is part of brisk-menu.
 *
 * Copyright © 2017 Ikey Doherty <ikey@solus-project.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "item.h"
#include "section.h"
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _BriskBackend BriskBackend;
typedef struct _BriskBackendClass BriskBackendClass;

/**
 * Flags to indicate the support level of a given backend
 */
typedef enum {
        BRISK_BACKEND_FAVOURITES = 1 << 0, /**<Supports "favourites" management */
        BRISK_BACKEND_KEYBOARD = 1 << 1,   /**<Supports keyboard shortcuts */
        BRISK_BACKEND_SOURCE = 1 << 2,     /**<Provides data which must be loaded */
} BriskBackendFlags;

struct _BriskBackendClass {
        GObjectClass parent_class;

        /* All plugins must implement these methods */
        unsigned int (*get_flags)(BriskBackend *);
        const gchar *(*get_id)(BriskBackend *);
        const gchar *(*get_display_name)(BriskBackend *);

        /* Favourites functions */
        gboolean (*pin_item)(BriskBackend *, BriskItem *);
        gboolean (*is_item_pinned)(BriskBackend *, BriskItem *);
        gboolean (*unpin_item)(BriskBackend *, BriskItem *);

        /* All plugins given an opportunity to load later in life */
        gboolean (*load)(BriskBackend *);

        /* Signals, gtk-doc style with param names */
        void (*item_added)(BriskBackend *backend, BriskItem *item);
        void (*item_removed)(BriskBackend *backend, const gchar *id);
        void (*section_added)(BriskBackend *backend, BriskSection *section);
        void (*section_removed)(BriskBackend *backend, const gchar *id);
        void (*reset)(BriskBackend *backend);

        gpointer padding[12];
};

/**
 * BriskBackend is an abstract top-level class which is used as the base
 * of all other backend implementations within Brisk.
 */
struct _BriskBackend {
        GObject parent;
};

#define BRISK_TYPE_BACKEND brisk_backend_get_type()
#define BRISK_BACKEND(o) (G_TYPE_CHECK_INSTANCE_CAST((o), BRISK_TYPE_BACKEND, BriskBackend))
#define BRISK_IS_BACKEND(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), BRISK_TYPE_BACKEND))
#define BRISK_BACKEND_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), BRISK_TYPE_BACKEND, BriskBackendClass))
#define BRISK_IS_BACKEND_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), BRISK_TYPE_BACKEND))
#define BRISK_BACKEND_GET_CLASS(o)                                                                 \
        (G_TYPE_INSTANCE_GET_CLASS((o), BRISK_TYPE_BACKEND, BriskBackendClass))

GType brisk_backend_get_type(void);

/* API Methods Follow */

/* Core vfuncs required by everyone */
unsigned int brisk_backend_get_flags(BriskBackend *backend);
const gchar *brisk_backend_get_id(BriskBackend *backend);
const gchar *brisk_backend_get_display_name(BriskBackend *backend);

/* Favourites specific functionality */
gboolean brisk_backend_pin_item(BriskBackend *backend, BriskItem *item);
gboolean brisk_backend_is_item_pinned(BriskBackend *backend, BriskItem *item);
gboolean brisk_backend_unpin_item(BriskBackend *backend, BriskItem *item);

/* Attempt to load for the first time */
gboolean brisk_backend_load(BriskBackend *backend);

/**
 * Helpers for subclasses
 */
void brisk_backend_item_added(BriskBackend *backend, BriskItem *item);
void brisk_backend_item_removed(BriskBackend *backend, const gchar *id);
void brisk_backend_section_added(BriskBackend *backend, BriskSection *section);
void brisk_backend_section_removed(BriskBackend *backend, const gchar *id);
void brisk_backend_reset(BriskBackend *backend);

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
