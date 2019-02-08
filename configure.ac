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

PKG_PROG_PKG_CONFIG

#AM_GNU_GETTEXT([external])
#AM_GNU_GETTEXT_VERSION([0.18.1])
#IT_PROG_INTLTOOL([0.40])

AC_CHECK_PROG(GLIB_COMPILE_RESOURCES, glib-compile-resources, yes)
if test x"$GLIB_COMPILE_RESOURCES" != x"yes" ; then
   AC_MSG_ERROR([glib-compile-resources not found.])
fi

GLIB_GSETTINGS

# Libraries

PKG_CHECK_MODULES([NUCOMMANDER], [gtkmm-3.0 >= 3.8.0])
PKG_CHECK_MODULES([ARCHGENERIC], [libarchive])
AX_BOOST_BASE


# intltool

#GETTEXT_PACKAGE=${PACKAGE_TARNAME}
#AC_SUBST([GETTEXT_PACKAGE])


# Output

AC_CONFIG_FILES([Makefile src/Makefile src/plugins/archgeneric/Makefile])
AC_OUTPUT
