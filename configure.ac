# Initialization
AC_INIT([NuCommander], [0.1], [alex.gutev@gmail.com])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AM_SILENT_RULES([yes])

LT_INIT


# Required Tools

AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11

AC_PROG_SED

PKG_PROG_PKG_CONFIG

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

AC_CHECK_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources, yes)
if test x"$GLIB_COMPILE_RESOURCES" != x"yes" ; then
   AC_MSG_ERROR([glib-compile-resources not found.])
fi

GLIB_GSETTINGS

# Libraries

PKG_CHECK_MODULES([GLIBMM], [gtkmm-3.0 >= 3.8.0])
PKG_CHECK_MODULES([LIBARCHIVE], [libarchive])
PKG_CHECK_MODULES([LIBLUA], [lua-5.2])
AX_BOOST_BASE

# Check for header files

AC_CHECK_HEADERS([stdlib.h dlfcn.h])
AC_SEARCH_LIBS([dlopen], [dl])


# Output

AC_CONFIG_FILES([Makefile src/Makefile src/plugins/archgeneric/Makefile po/Makefile.in])
AC_OUTPUT
