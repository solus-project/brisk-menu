AM_CFLAGS = \
	-fstack-protector -Wall -pedantic			\
	-Wstrict-prototypes -Wundef -fno-common 		\
	-Werror-implicit-function-declaration 			\
	-Wformat -Wformat-security -Werror=format-security 	\
	-Wconversion -Wunused-variable -Wunreachable-code 	\
	-Wall -W -D_FORTIFY_SOURCE=2 -std=c11			\
	-DGDK_VERSION_MAX_ALLOWED=GDK_VERSION_3_18 		\
	-DGDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_18		\
	-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_44		\
	-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_44

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include

noinst_PROGRAMS = \
	brisk-menu-test

brisk_resources = $(shell glib-compile-resources --sourcedir=$(top_srcdir)/data --generate-dependencies $(top_srcdir)/data/brisk.gresource.xml)

BUILT_SOURCES = \
	brisk-resources.h \
	brisk-resources.c

CLEANFILES = \
	$(BUILT_SOURCES)

# resources
brisk-resources.h: $(top_srcdir)/data/brisk.gresource.xml $(theme_resources)
	glib-compile-resources --target=$@ --sourcedir=$(top_srcdir)/data --generate-header --c-name brisk_resources $<

brisk-resources.c: $(top_srcdir)/data/brisk.gresource.xml $(theme_resources)
	glib-compile-resources --target=$@ --sourcedir=$(top_srcdir)/data --generate-source --c-name brisk_resources $<


# Helper to let us develop the window portion first
brisk_menu_test_SOURCES = \
	brisk-resources.h \
	brisk-resources.c \
	src/applet.h \
	src/applet.c \
	src/main.c \
	src/category-button.h \
	src/category-button.c \
	src/desktop-button.h \
	src/desktop-button.c \
	src/entry-button.h \
	src/entry-button.c \
	src/launcher.h \
	src/launcher.c \
	src/menu-private.h \
	src/menu-search.c \
	src/menu-sort.c \
	src/menu-loader.c \
	src/menu-window.h \
	src/menu-window.c

brisk_menu_test_CFLAGS = \
	$(GTK_CFLAGS) \
	$(MENU_CFLAGS) \
	$(APPLET_CFLAGS) \
	$(AM_CFLAGS)

brisk_menu_test_LDADD = \
	$(GTK_LIBS) \
	$(MENU_LIBS) \
	$(APPLET_LIBS)
