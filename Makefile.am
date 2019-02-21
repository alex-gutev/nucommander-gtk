ACLOCAL_AMFLAGS = -I m4
SUBDIRS = . src src/plugins/archgeneric po

desktopdir = $(datadir)/applications
dist_desktop_in_files = nucommander.desktop.in
dist_desktop_DATA = $(dist_desktop_in_files:.desktop.in=.desktop)

nobase_dist_pkgdata_DATA = commands/open-terminal.lua commands/view.lua

CLEANFILES = $(srcdir)/nucommander.desktop

nucommander.desktop: nucommander.desktop.in po/nucommander.pot po/bg.po
	$(AM_V_GEN) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $(srcdir)/$@
