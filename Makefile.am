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
	-I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/lib

EXTRA_DIST = \
	autogen.sh \
	AUTHORS \
	LICENSE \
	LICENSE.CC-BY-SA-4.0 \
	data/brisk.gresource.xml \
	data/org.mate.panel.applet.BriskMenuFactory.service.in \
	data/com.solus_project.brisk.BriskMenu.mate-panel-applet.in \
	data/com.solus-project.brisk-menu.gschema.xml \
	data/styling.css \
	data/brisk_system-log-out-symbolic.svg \
	src/frontend/org.mate.ScreenSaver.xml \
	src/frontend/org.gnome.SessionManager.xml

iconsdir = \
	$(datadir)/icons/hicolor/scalable/actions

icons_DATA = \
	data/brisk_system-log-out-symbolic.svg

libexec_PROGRAMS = \
	brisk-menu

noinst_PROGRAMS = \
	brisk-test-backends

noinst_LTLIBRARIES = \
	libbriskresources.la \
	libbriskui.la \
	libbriskutil.la \
	libbriskbackend.la

brisk_resources = $(shell glib-compile-resources --sourcedir=$(top_srcdir)/data --generate-dependencies $(top_srcdir)/data/brisk.gresource.xml)

# Generate the glue
libsession-glue.h: $(top_srcdir)/src/frontend/org.gnome.SessionManager.xml
	gdbus-codegen --interface-prefix org.gnome. \
	--c-namespace Gnome  \
	--generate-c-code libsession-glue $<

libsession-glue.c: libsession-glue.h

# Generate the screensaver glue
libsaver-glue.h: $(top_srcdir)/src/frontend/org.mate.ScreenSaver.xml
	gdbus-codegen --interface-prefix org.mate. \
	--c-namespace Mate \
	--generate-c-code libsaver-glue $<

libsaver-glue.c: libsaver-glue.h

# Build the stub first to compile into the main session library
noinst_LTLIBRARIES += \
	libsession-stub.la

libsession_stub_la_SOURCES = \
	libsession-glue.c \
	libsession-glue.h \
	libsaver-glue.h \
	libsaver-glue.c

libsession_stub_la_CFLAGS = \
	$(GIO_CFLAGS)

libsession_stub_la_LIBS = \
	$(GIO_LIBS)

BUILT_SOURCES = \
	brisk-resources.h \
	brisk-resources.c \
	libsession-glue.h \
	libsession-glue.c \
	libsaver-glue.h \
	libsaver-glue.c

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

libbriskutil_la_SOURCES = \
	src/lib/authors.h \
	src/lib/key-binder.h \
	src/lib/key-binder.c \
	src/lib/styles.h \
	src/lib/util.h

libbriskutil_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(X11_CFLAGS) \
	$(AM_CFLAGS)

libbriskutil_la_LIBADD = \
	$(GTK_LIBS) \
	$(X11_LIBS)

# Backend management
libbriskbackend_la_SOURCES = \
	src/backend/backend.h \
	src/backend/backend.c \
	src/backend/item.h \
	src/backend/item.c \
	src/backend/section.h \
	src/backend/section.c \
	src/backend/all-items/all-backend.h \
	src/backend/all-items/all-backend.c \
	src/backend/all-items/all-section.h \
	src/backend/all-items/all-section.c \
	src/backend/apps/apps-backend.h \
	src/backend/apps/apps-backend.c \
	src/backend/apps/apps-item.h \
	src/backend/apps/apps-item.c \
	src/backend/apps/apps-section.h \
	src/backend/apps/apps-section.c \
	src/backend/favourites/favourites-backend.h \
	src/backend/favourites/favourites-backend.c \
	src/backend/favourites/favourites-desktop.c \
	src/backend/favourites/favourites-section.h \
	src/backend/favourites/favourites-section.c

libbriskbackend_la_CFLAGS = \
	$(GIO_CFLAGS) \
	$(MENU_CFLAGS) \
	$(AM_CFLAGS)

libbriskbackend_la_LIBADD = \
	$(GIO_LIBS) \
	$(MENU_LIBS)

libbriskui_la_SOURCES = \
	src/frontend/category-button.h \
	src/frontend/category-button.c \
	src/frontend/desktop-button.h \
	src/frontend/desktop-button.c \
	src/frontend/entry-button.h \
	src/frontend/entry-button.c \
	src/frontend/launcher.h \
	src/frontend/launcher.c \
	src/frontend/menu-context.c \
	src/frontend/menu-grabs.c \
	src/frontend/menu-keyboard.c \
	src/frontend/menu-loader.c \
	src/frontend/menu-private.h \
	src/frontend/menu-search.c \
	src/frontend/menu-session.c \
	src/frontend/menu-settings.c \
	src/frontend/menu-sort.c \
	src/frontend/menu-window.h \
	src/frontend/menu-window.c \
	src/frontend/sidebar-scroller.h \
	src/frontend/sidebar-scroller.c

libbriskui_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(APPLET_CFLAGS) \
	$(X11_CFLAGS) \
	$(MENU_CFLAGS) \
	$(AM_CFLAGS)

libbriskui_la_LIBADD = \
	libbriskbackend.la \
	$(GTK_LIBS) \
	$(X11_LIBS) \
	$(MENU_LIBS)

# Helper to let us develop the window portion first
brisk_menu_SOURCES = \
	src/mate-applet/applet.h \
	src/mate-applet/applet.c \
	src/mate-applet/applet-position.c \
	src/mate-applet/main.c

brisk_menu_CFLAGS = \
	$(GTK_CFLAGS) \
	$(APPLET_CFLAGS) \
	$(MENU_CFLAGS) \
	$(AM_CFLAGS)

brisk_menu_LDADD = \
	libbriskresources.la \
	libbriskbackend.la \
	libbriskui.la \
	libbriskutil.la \
	libsession-stub.la \
	$(GTK_LIBS) \
	$(X11_LIBS) \
	$(MENU_LIBS) \
	$(APPLET_LIBS)

brisk_test_backends_SOURCES = \
	src/test/brisk-test-backends.c

brisk_test_backends_CFLAGS = \
	$(GTK_CFLAGS) \
	$(MENU_CFLAGS) \
	$(AM_CFLAGS)

brisk_test_backends_LDADD = \
	libbriskbackend.la \
	libbriskutil.la \
	$(GTK_LIBS) \
	$(MENU_LIBS)

gsettings_SCHEMAS = \
	data/com.solus-project.brisk-menu.gschema.xml

@GSETTINGS_RULES@

SUBDIRS = \
	. \
	po

