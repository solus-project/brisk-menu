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

EXTRA_DIST = \
	data/brisk.gresource.xml \
	data/org.mate.panel.applet.BriskMenuFactory.service.in \
	data/com.solus_project.brisk.BriskMenu.mate-panel-applet.in \
	data/styling.css

libexec_PROGRAMS = \
	brisk-menu

noinst_LTLIBRARIES = \
	libbriskresources.la

brisk_resources = $(shell glib-compile-resources --sourcedir=$(top_srcdir)/data --generate-dependencies $(top_srcdir)/data/brisk.gresource.xml)

BUILT_SOURCES = \
	brisk-resources.h \
	brisk-resources.c

CLEANFILES = \
	$(BUILT_SOURCES) \
	data/com.solus_project.brisk.BriskMenu.mate-panel-applet \
	data/org.mate.panel.applet.BriskMenuFactory.service

# Borrowed/adapted from mate-panel
%.mate-panel-applet: %.mate-panel-applet.in
	$(AM_V_GEN) \
	$(MKDIR_P) "$(dir $@)"; \
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
	    $< > $@

# Also borrowed/adapted from mate-panel
%.service: %.service.in
	$(AM_V_GEN) \
	$(MKDIR_P) "$(dir $@)"; \
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
	    $< > $@

appletdir = $(datadir)/mate-panel/applets
applet_DATA = \
	data/com.solus_project.brisk.BriskMenu.mate-panel-applet

servicedir = $(datadir)/dbus-1/services
service_DATA = \
	data/org.mate.panel.applet.BriskMenuFactory.service

# resources
brisk-resources.h: $(top_srcdir)/data/brisk.gresource.xml $(theme_resources)
	glib-compile-resources --manual-register --target=$@ --sourcedir=$(top_srcdir)/data --generate-header --c-name brisk_resources $<

brisk-resources.c: $(top_srcdir)/data/brisk.gresource.xml $(theme_resources)
	glib-compile-resources --manual-register --target=$@ --sourcedir=$(top_srcdir)/data --generate-source --c-name brisk_resources $<

# Build this separately due to major spammy compiler warnings
libbriskresources_la_SOURCES = \
	brisk-resources.h \
	brisk-resources.c

libbriskresources_la_CFLAGS = \
	$(GIO_CFLAGS)

libbriskresources_la_LIBADD = \
	$(GIO_LIBS)

# Helper to let us develop the window portion first
brisk_menu_SOURCES = \
	src/applet.h \
	src/applet.c \
	src/category-button.h \
	src/category-button.c \
	src/desktop-button.h \
	src/desktop-button.c \
	src/entry-button.h \
	src/entry-button.c \
	src/key-binder.h \
	src/key-binder.c \
	src/launcher.h \
	src/launcher.c \
	src/menu-grabs.c \
	src/menu-keyboard.c \
	src/menu-private.h \
	src/menu-search.c \
	src/menu-session.c \
	src/menu-sort.c \
	src/menu-loader.c \
	src/menu-window.h \
	src/menu-window.c \
	src/util.h

brisk_menu_CFLAGS = \
	$(GTK_CFLAGS) \
	$(X11_CFLAGS) \
	$(MENU_CFLAGS) \
	$(APPLET_CFLAGS) \
	$(AM_CFLAGS)

brisk_menu_LDADD = \
	libbriskresources.la \
	$(GTK_LIBS) \
	$(X11_LIBS) \
	$(MENU_LIBS) \
	$(APPLET_LIBS)
