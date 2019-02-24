# Initialization
AC_INIT([NuCommander], [0.1], [alex.gutev@gmail.com])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AM_SILENT_RULES([yes])

LT_INIT


## Required Tools

# Compilers

AC_PROG_CC
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11

# pkg-config

PKG_PROG_PKG_CONFIG

# Misc Tools

AC_PROG_SED

# gettext

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

# glib-compile-resources

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


# OS X App bundle

AC_ARG_ENABLE([app-bundle], AS_HELP_STRING([--enable-app-bundle], [Enable App Bundle Creation]))

AS_IF([test "x$enable_app_bundle" == "xyes"],

      [AC_CHECK_PROG(has_gtk_mac_bundler, gtk-mac-bundler, yes)
       AS_IF([test "x$has_gtk_mac_bundler" != "xyes"],
             [AC_MSG_ERROR([gtk-mac-bundler not found. Required to create App bundle.])])
       AC_SUBST([nucommander_install_datadir], [Resources/share/nucommander])
       AC_CONFIG_FILES([bundle/nucommander.bundle])],

      [AC_SUBST([nucommander_install_datadir], [${datadir}/${PACKAGE}])])

AC_ARG_VAR(BUNDLE_INSTALL_DIR, [Path to the directory where the App bundle is to be installed])
AC_SUBST([GTKMM_PREFIX], [$(${PKG_CONFIG} --variable=prefix gtkmm-3.0)])


# Output

AC_CONFIG_FILES([Makefile src/Makefile src/plugins/archgeneric/Makefile po/Makefile.in])
AC_OUTPUT
