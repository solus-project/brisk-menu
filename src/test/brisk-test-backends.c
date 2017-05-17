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

#define _GNU_SOURCE

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

BRISK_BEGIN_PEDANTIC
#include "backend/apps/apps-backend.h"
BRISK_END_PEDANTIC

DEF_AUTOFREE(BriskBackend, g_object_unref)
DEF_AUTOFREE(char, free)

/**
 * Mimic functionality from check library
 */
static inline void fail_if(bool b, const char *fmt, ...)
{
        va_list va;
        autofree(char) *out = NULL;

        if (!b) {
                return;
        }

        va_start(va, fmt);

        if (vasprintf(&out, fmt, va) < 0) {
                fputs("Out of memory\n", stderr);
                exit(1);
        }

        fprintf(stderr, " => error: %s\n", out);
        va_end(va);
        exit(1);
}

static void test_item_added(__brisk_unused__ BriskBackend *backend, BriskItem *item,
                            __brisk_unused__ gpointer v)
{
        g_message("Got a new item: %s \"%s\"", brisk_item_get_id(item), brisk_item_get_name(item));
}

static void test_section_added(__brisk_unused__ BriskBackend *backend, BriskSection *section,
                               __brisk_unused__ gpointer v)
{
        fputs("\n", stdout);
        g_message("Got a new section: %s \"%s\"\n",
                  brisk_section_get_id(section),
                  brisk_section_get_name(section));
}

/**
 * Test the applications backend
 */
static void run_apps_backend_test(BriskBackend *backend, GMainLoop *loop)
{
        fail_if((brisk_backend_get_flags(backend) & BRISK_BACKEND_SOURCE) != BRISK_BACKEND_SOURCE,
                "Invalid flags for backend");

        g_signal_connect(backend, "item-added", G_CALLBACK(test_item_added), NULL);
        g_signal_connect(backend, "section-added", G_CALLBACK(test_section_added), NULL);

        fail_if(!brisk_backend_load(backend), "Failed to load the apps backend");

        /* Stop processing after 5 seconds */
        g_timeout_add_seconds(5, (GSourceFunc)g_main_loop_quit, loop);
}

int main(__brisk_unused__ int argc, __brisk_unused__ char **argv)
{
        GMainLoop *loop = NULL;
        autofree(BriskBackend) *backend = NULL;
        backend = brisk_apps_backend_new();
        fail_if(backend == NULL, "Failed to construct apps backend");

        loop = g_main_loop_new(NULL, FALSE);
        run_apps_backend_test(backend, loop);
        g_main_loop_run(loop);
        g_main_loop_unref(loop);
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
