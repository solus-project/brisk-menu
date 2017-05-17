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

/**
 * Test the applications backend
 */
static void run_apps_backend_test(void)
{
        autofree(BriskBackend) *backend = NULL;

        backend = brisk_apps_backend_new();
        fail_if(backend == NULL, "Failed to construct apps backend");

        fail_if((brisk_backend_get_flags(backend) & BRISK_BACKEND_SOURCE) != BRISK_BACKEND_SOURCE,
                "Invalid flags for backend");

        fail_if(!brisk_backend_load(backend), "Failed to load the apps backend");
}

int main(__brisk_unused__ int argc, __brisk_unused__ char **argv)
{
        run_apps_backend_test();
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
