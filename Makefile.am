ACLOCAL_AMFLAGS = -I m4
SUBDIRS = . src src/plugins/archgeneric po test

nobase_dist_pkgdata_DATA = commands/open-terminal.lua commands/view.lua

# Desktop File

desktopdir = $(datadir)/applications
dist_desktop_in_files = nucommander.desktop.in
dist_desktop_DATA = $(dist_desktop_in_files:.desktop.in=.desktop)

CLEANFILES = $(srcdir)/nucommander.desktop

nucommander.desktop: nucommander.desktop.in po/nucommander.pot po/bg.po
	$(AM_V_GEN) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $(srcdir)/$@


# GSettings

nucommander.gschema.xml: nucommander.gschema.xml.in
	$(AM_V_GEN) $(SED) -e 's|@libdir[@]|$(nucommander_install_libdir)|g' $< > $@

gsettings_SCHEMAS = nucommander.gschema.xml


EXTRA_DIST = nucommander.gschema.xml.in
CLEANFILES += nucommander.gschema.xml

@GSETTINGS_RULES@


# Mac App Bundle

bundle/nucommander.bundle: bundle/nucommander.bundle.in
	$(AM_V_GEN) $(MKDIR_P) bundle && $(SED) \
	-e 's|@prefix[@]|$(DESTDIR)$(prefix)|g' \
	-e 's|@GTKMM_PREFIX[@]|$(GTKMM_PREFIX)|g' \
	-e 's|@BUNDLE_INSTALL_DIR[@]|$(BUNDLE_INSTALL_DIR)|g' \
	-e 's|@srcdir[@]|$(srcdir)|g' \
	$< > $@

app-bundle: bundle/nucommander.bundle $(srcdir)/bundle/info.plist $(srcdir)/bundle/launcher.sh install
	gtk-mac-bundler bundle/nucommander.bundle

uninstall-app-bundle:
	rm -rf $(BUNDLE_INSTALL_DIR)/NuCommander.app

CLEANFILES += bundle/nucommander.bundle

.PHONY: app-bundle uninstall-app-bundle
