AC_INIT([Brisk Menu], 0.3.0, [ikey@solus-project.com], [brisk-menu], [https://solus-project.com/])
AM_INIT_AUTOMAKE([-Wno-portability no-dist-gzip dist-xz foreign subdir-objects 1.9 tar-ustar])
AC_PROG_CC
AC_PROG_CC_STDC
AC_PREFIX_DEFAULT(/usr/local)
AM_SILENT_RULES([yes])
AC_CONFIG_MACRO_DIR([m4])
LT_PREREQ(2.2)
LT_INIT([disable-static])

GLIB_GSETTINGS

# Source: https://mail.gnome.org/archives/commits-list/2012-September/msg01730.html
# adl_RECURSIVE_EVAL(VALUE, RESULT)
# =================================
# Interpolate the VALUE in loop until it doesn't change,
# and set the result to $RESULT.
# WARNING: It's easy to get an infinite loop with some unsane input.
# For example ${datadir} becomes ${datarootdir}, and then ${prefix}/share, and
# finally ${prefix} is replaced by the prefix.
AC_DEFUN([adl_RECURSIVE_EVAL],
[_lcl_receval="$1"
$2=`(test "x$prefix" = xNONE && prefix="$ac_default_prefix"
     test "x$exec_prefix" = xNONE && exec_prefix="${prefix}"
     _lcl_receval_old=''
     while test "[$]_lcl_receval_old" != "[$]_lcl_receval"; do
       _lcl_receval_old="[$]_lcl_receval"
       eval _lcl_receval="\"[$]_lcl_receval\""
     done
     echo "[$]_lcl_receval")`])

# Build requirements
m4_define([gtk_required_version], [3.18.0])
m4_define([glib_required_version], [2.44.0])
m4_define([mate_required_version], [1.16.0])

PKG_CHECK_MODULES(GTK,
                 [
                   gtk+-3.0 >= gtk_required_version
                 ]
)

PKG_CHECK_MODULES(X11,
                 [
                   gdk-x11-3.0 >= gtk_required_version
                   x11
                 ]
)

PKG_CHECK_MODULES(GIO,
                 [
                   gobject-2.0 >= glib_required_version
                   gio-unix-2.0 >= glib_required_version
                 ]
)

PKG_CHECK_MODULES(MENU,
                 [
                   libmate-menu >= mate_required_version
                 ]
)

PKG_CHECK_MODULES(APPLET,
                 [
                   libmatepanelapplet-4.0 >= mate_required_version
                 ]
)

AC_CONFIG_FILES([Makefile
                 po/Makefile.in])

IT_PROG_INTLTOOL([0.50.0])
GETTEXT_PACKAGE=brisk-menu
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",
                   [The prefix for our gettext translation domains.])


adl_RECURSIVE_EVAL(["${datadir}/locale"], [MATELOCALEDIR])
AC_SUBST(MATELOCALEDIR)
AC_DEFINE_UNQUOTED(MATELOCALEDIR, "$MATELOCALEDIR", [locale directory])
# Force into config.h for MATE macros
AC_DEFINE([ENABLE_NLS], [1])

AC_CONFIG_HEADERS([config.h])

AC_OUTPUT

AM_GLIB_GNU_GETTEXT
AC_MSG_RESULT([
        brisk-menu $VERSION
        =================

        prefix:                 ${prefix}
        libdir:                 ${libdir}
        sysconfdir:             ${sysconfdir}
        exec_prefix:            ${exec_prefix}
        bindir:                 ${bindir}
        datarootdir:            ${datarootdir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}
])
