bin_PROGRAMS = nucommander
nucommander_LDADD = $(GTKMM_LIBS) $(LIBLUA_LIBS)

nucommander_CFLAGS = $(GTKMM_CFLAGS) $(LIBLUA_CFLAGS)
nucommander_CXXFLAGS = $(GTKMM_CFLAGS) $(BOOST_CPPFLAGS) $(LIBLUA_CFLAGS)
nucommander_CPPFLAGS = -DLOCALEDIR=\""$(localedir)"\"

## Source Files

nucommander_SOURCES = \
	main.cpp \
	nucommander.cpp \
	nucommander.h \
	types.h \
	paths/pathname.h \
	paths/pathname.cpp \
	plugins/archive_plugin_types.h \
	plugins/archive_plugin.h \
	plugins/archive_plugin.cpp \
	plugins/archive_plugin_loader.h \
	plugins/archive_plugin_loader.cpp \
	errors/errors.h \
	errors/error.h \
	errors/error.cpp \
	errors/file_error.h \
	errors/file_error.cpp \
	errors/attribute_error.h \
	errors/attribute_error.cpp \
	errors/restarts.h \
	errors/restarts.cpp \
	errors/error_dialog.h \
	errors/error_dialog.cpp \
	tasks/async_queue.h \
	tasks/async_task.h \
	tasks/async_task.cpp \
	tasks/cancel_state.h \
	tasks/cancel_state.cpp \
	tasks/task_queue.h \
	tasks/task_queue.cpp \
	tasks/progress.h \
	lister/lister.h \
	lister/dir_lister.h \
	lister/dir_lister.cpp \
	lister/archive_lister.h \
	lister/archive_lister.cpp \
	lister/sub_archive_lister.h \
	lister/sub_archive_lister.cpp \
	lister/tree_lister.h \
	lister/tree_lister.cpp \
	lister/dir_tree_lister.h \
	lister/dir_tree_lister.cpp \
	lister/archive_tree_lister.h \
	lister/archive_tree_lister.cpp \
	directory/vfs.h \
	directory/vfs.cpp \
	directory/dir_monitor.h \
	directory/dir_monitor.cpp \
	directory/dir_type.h \
	directory/dir_type.cpp \
	directory/archive_tree.h \
	directory/archive_tree.cpp \
	directory/icon_loader.h \
	directory/icon_loader.cpp \
	directory/dir_tree.h \
	directory/dir_tree.cpp \
	directory/dir_entry.h \
	directory/dir_entry.cpp \
	stream/error_macros.h \
	stream/instream.h \
	stream/instream.cpp \
	stream/file_instream.h \
	stream/file_instream.cpp \
	stream/archive_instream.h \
	stream/archive_instream.cpp \
	stream/outstream.h \
	stream/file_outstream.h \
	stream/file_outstream.cpp \
	stream/archive_outstream.h \
	stream/archive_outstream.cpp \
	stream/dir_writer.h \
	stream/reg_dir_writer.h \
	stream/reg_dir_writer.cpp \
	stream/archive_dir_writer.h \
	stream/archive_dir_writer.cpp \
	stream/sub_archive_dir_writer.h \
	stream/sub_archive_dir_writer.cpp \
	operations/copy.h \
	operations/copy.cpp \
	operations/move.h \
	operations/move.cpp \
	operations/delete.h \
	operations/delete.cpp \
	operations/dir_size.h \
	operations/dir_size.cpp \
	commands/commands.h \
	commands/commands.cpp \
	commands/builtin.h \
	commands/builtin.cpp \
	commands/custom_commands.h \
	commands/custom_commands.cpp \
	commands/lua_command.h \
	commands/lua_command.cpp \
	commands/lua_api.h \
	commands/lua_api.cpp \
	luawrapper/luawrapper.hpp \
	settings/app_settings.h \
	settings/app_settings.cpp \
	settings/variant.h \
	interface/dest_dialog.h \
	interface/dest_dialog.cpp \
	interface/progress_dialog.h \
	interface/progress_dialog.cpp \
	interface/prefs_window.h \
	interface/prefs_window.cpp \
	interface/open_dirs_popup.h \
	interface/open_dirs_popup.cpp \
	interface/app_window.cpp \
	interface/app_window.h \
	interface/file_view.h \
	interface/file_view.cpp \
	util/util.h \
	file_list/list_controller.h \
	file_list/file_list_controller.h \
	file_list/file_list_controller.cpp \
	file_list/filtered_list_controller.h \
	file_list/filtered_list_controller.cpp \
	file_list/file_model_columns.h \
	file_list/file_model_columns.cpp \
	file_list/columns.h \
	file_list/columns.cpp \
	file_list/sort_func.h \
	file_list/sort_func.cpp \
	file_list/directory_buffers.h \
	file_list/directory_buffers.cpp \
	search/fuzzy_filter.h

## Resources

nodist_nucommander_SOURCES = resources.c

nucommander_RESOURCES = \
	resources/window.ui \
	resources/fileview.ui \
	resources/error_dialog.ui \
	resources/dest_dialog.ui \
	resources/progress_dialog.ui \
	resources/prefs_window.ui \
	resources/open_dirs_popup.ui \
	resources/main_menu.ui \
	resources/nucommander.gresource.xml \
	resources/styles.css \
	resources/license.txt


$(srcdir)/resources.c: $(nucommander_RESOURCES)
	$(AM_V_GEN) glib-compile-resources --target=$(srcdir)/resources.c --sourcedir=$(srcdir)/resources --generate-source $(srcdir)/resources/nucommander.gresource.xml

EXTRA_DIST = $(nucommander_RESOURCES)
CLEANFILES = $(srcdir)/resources.c
