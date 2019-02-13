ACLOCAL_AMFLAGS = -I m4
SUBDIRS = . src src/plugins/archgeneric po

desktopdir = $(datadir)/applications
dist_desktop_in_files = nucommander.desktop.in
dist_desktop_DATA = $(dist_desktop_in_files:.desktop.in=.desktop)

CLEANFILES = $(srcdir)/nucommander.desktop

nucommander.desktop: nucommander.desktop.in po/nucommander.pot po/bg.po
	$(AM_V_GEN) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $(srcdir)/$@

osx-set-defaults:
	defaults write org.agware.nucommander "/org/agware/NuCommander/archgeneric/path" '"Resources/lib/nucommander/libarchgeneric.so"'
	defaults write org.agware.nucommander "/org/agware/NuCommander/archgeneric/regex" '".*?(?:.zip|.tar|.tar.gz)$$"'
	defaults write org.agware.nucommander "/org/agware/NuCommander/plugins" -array '"archgeneric"'

osx-app-bundle: nucommander.bundle $(srcdir)/info.plist $(srcdir)/launcher.sh install osx-set-defaults
	gtk-mac-bundler nucommander.bundle

uninstall-osx-app-bundle:
	rm -rf $(BUNDLE_INSTALL_DIR)/NuCommander.app

.PHONY: osx-app-bundle
