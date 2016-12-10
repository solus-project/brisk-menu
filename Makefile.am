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
	solmenu-test

# Helper to let us develop the window portion first
solmenu_test_SOURCES = \
	src/main.c \
	src/menu-private.h \
	src/menu-loader.c \
	src/menu-window.h \
	src/menu-window.c

solmenu_test_CFLAGS = \
	$(GTK_CFLAGS) \
	$(MENU_CFLAGS) \
	$(AM_CFLAGS)

solmenu_test_LDADD = \
	$(GTK_LIBS) \
	$(MENU_LIBS)
