/*
 * NuCommander
 * Copyright (C) 2019  Alexander Gutev <alex.gutev@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "builtin.h"

#include <glib/gi18n.h>
#include <gtkmm/messagedialog.h>

#include "nucommander.h"
#include "interface/app_window.h"
#include "interface/file_view.h"

#include "interface/prefs_window.h"
#include "interface/dest_dialog.h"

#include "operations/copy.h"
#include "operations/move.h"
#include "operations/delete.h"

#include "file_list/directory_buffers.h"

#include "util/util.h"

using namespace nuc;


//// Utility Function Prototypes

/**
 * Expands relative destination paths to absolute paths relative to @a
 * path.
 *
 * If @a dest is a relative path it is expanded relative to @a path,
 * otherwise it is returned.
 *
 * @param The absolute path, relative to which, relative paths are
 *   expanded.
 *
 * @param The destination path.
 *
 * @return The absolute relative destination path.
 */
static pathname expand_dest_path(const pathname &path, const pathname &dest);


//// Builtin Commands

/**
 * Copy Command.
 *
 * Queries the user for a destination directory, via the destination
 * dialog, and copies the selected/marked files in the source pane to
 * the directory.
 */
struct copy_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Copy marked/selected files, in source pane, to a destination directory.");
    }
};

/**
 * Make Directory Command.
 *
 * Displays a destination path dialog which queries the user for the
 * name of the directory to create.
 *
 * Adds a task to the window's task queue which creates the directory
 * in the source path.
 */
struct make_dir_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    /**
     * Make directory task function. Creates the directory @a name in the
     * directory @a dest.
     *
     * @param state Task cancellation state.
     * @param dest The directory in which to create the directory.
     * @param name Name of the directory to create.
     */
    static void make_dir_task(nuc::cancel_state &state, const nuc::pathname::string &dest, const nuc::pathname::string &name);

    virtual std::string description() const noexcept {
        return _("Create a new directory in the source pane.");
    }
};

/**
 * Move Command.
 *
 * Queries the user for a destination directory, via the destination
 * dialog, and moves the selected/marked files in the source pane, @a
 * src, to the that directory.
 */
struct move_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Move/Rename marked/selected files in the source pane.");
    }
};

/**
 * Delete Command.
 *
 * Deletes the selected files in the source pane, after querying the
 * user for confirmation.
 */
struct delete_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    /**
     * Creates the delete confirmation message.
     *
     * @param ents Array of selected entries to be deleted.
     *
     * @return The confirmation message.
     */
    static Glib::ustring confirm_delete_msg(const std::vector<dir_entry *> &ents);

    virtual std::string description() const noexcept {
        return _("Delete the marked/selected files in the source pane");
    }
};

/**
 * Change Path Command.
 *
 * Moves the keyboard focus to the current path entry in order to
 * change the current path.
 */
struct change_path_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Change the current directory path in the source pane.");
    }
};

/**
 * Begin Filter Command.
 *
 * Shows the filter entry in the source pane.
 */
struct begin_filter_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Begin filtering in the source pane.");
    }
};

/**
 * Begin filter with character command.
 *
 * Begins filtering and sets the character, corresponding to the event
 * if any, as the contents of the filter text entry.
 */
struct begin_filter_type_command : public begin_filter_command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);
};

/**
 * Open Preferences Window Command.
 */
struct preferences_command : public command {
    virtual void run(nuc::app_window *, nuc::file_view *, const GdkEventAny *, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Open the keybinding preferences.");
    }
};

/**
 * Swap Panes Command.
 *
 * Swaps the left and right file list panes.
 */
struct swap_panes_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Swap source and destination panes' directories.");
    }
};

/**
 * Change Directory Command.
 *
 * Displays the open directory list popup.
 */
struct change_dir_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Display the list of open directories, to choose a new directory to display in the source pane.");
    }
};

/**
 * Create New Open Directory Command.
 *
 * Creates a new file_list_controller and sets it as the
 * file_list_controller of the source pane.
 */
struct open_dir_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Create a new directory buffer.");
    }
};

/**
 * Close Directory Buffer.
 *
 * Deletes the file_list_controller of the source pane and switches to
 * the file_list_controller at the top of the stack.
 */
struct close_dir_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Close the source pane's current directory and return to the previously visited directory.");
    }
};

/**
 * Cancel Read Directory.
 *
 * Cancels ongoing read tasks in the source pane.
 */
struct cancel_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Cancel reading the directory in the source pane");
    }
};

/**
 * Quit Command.
 *
 * Quits the application.
 */
struct quit_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase);

    virtual std::string description() const noexcept {
        return _("Quit the application");
    }
};


//// Implementation

void nuc::add_builtin_commands(command_keymap::command_map &table) {
    table.emplace("copy", std::make_shared<copy_command>());
    table.emplace("make-directory", std::make_shared<make_dir_command>());
    table.emplace("move", std::make_shared<move_command>());
    table.emplace("delete", std::make_shared<delete_command>());
    table.emplace("change-path", std::make_shared<change_path_command>());
    table.emplace("begin-filter", std::make_shared<begin_filter_command>());
    table.emplace("begin-filter-type", std::make_shared<begin_filter_type_command>());
    table.emplace("preferences", std::make_shared<preferences_command>());
    table.emplace("swap-panes", std::make_shared<swap_panes_command>());
    table.emplace("change-directory", std::make_shared<change_dir_command>());
    table.emplace("open-new-directory", std::make_shared<open_dir_command>());
    table.emplace("close-directory", std::make_shared<close_dir_command>());
    table.emplace("cancel", std::make_shared<cancel_command>());
    table.emplace("quit", std::make_shared<quit_command>());
}


/// Utility Functions

pathname expand_dest_path(const pathname &path, const pathname &dest) {
    return path.ensure_dir(true).merge(dest);
}

/**
 * Adds a task to the window's task queue.
 *
 * @param window The window.
 *
 * @param src The source pane.
 *
 * @param response The response code of the dialog. If equal to
 *   Gtk::RESPONSE_OK the task is added to the window's task queue.
 *
 * @param f A function which should return the task to add to the
 *   queue.
 */
template <typename F>
void add_window_task(nuc::app_window *window, nuc::file_view *src, int response, F f) {
    if (response == Gtk::RESPONSE_OK) {
        auto type = src->dir_vfs()->directory_type();
        window->add_operation(f(), window->get_progress_fn(type));
    }
}


//// Copy Command Implementation

void copy_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    if (window && src) {
        auto entries = src->selected_entries();

        if (!entries.empty()) {
            dest_dialog *dialog = window->dest_dialog();

            dialog->set_query_label(_("Destination"));
            dialog->dest_path(src->next_file_view->path().ensure_dir(true).path());
            dialog->set_exec_button_label(_("Copy"));
            dialog->set_title(_("Copy"));

            add_window_task(window, src, dialog->run(), [=] {
                return make_copy_task(src->dir_vfs()->directory_type(),
                                      entries,
                                      expand_dest_path(src->path(), dialog->dest_path()));
            });
        }
    }
}


//// Make Directory Command Implementation

void make_dir_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    using namespace std::placeholders;

    if (window && src) {
        pathname::string dest = src->path();

        dest_dialog *dialog = window->dest_dialog();

        dialog->set_query_label(_("Directory Name"));
        dialog->dest_path("");
        dialog->set_exec_button_label(_("Create"));
        dialog->set_title(_("New Directory"));

        add_window_task(window, src, dialog->run(), [=] {
            return std::bind(make_dir_task, _1, dest, dialog->dest_path());
        });
    }
}

void make_dir_command::make_dir_task(cancel_state &state, const pathname::string &dest, const pathname::string &name) {
    state.call_progress(progress_event(progress_event::type_begin));

    try {
        std::unique_ptr<dir_writer> writer(dir_type::get_writer(dest));

        writer->mkdir(name, false);
        writer->close();
    }
    catch (const error &e) {
        // Catch error to abort operation
    }

    state.call_progress(progress_event(progress_event::type_finish));
}


//// Move Command Implementation

void move_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    if (window && src) {
        auto entries = src->selected_entries();

        if (!entries.empty()) {
            dest_dialog *dialog = window->dest_dialog();

            dialog->set_query_label(_("Destination"));
            dialog->dest_path(pathname(src->next_file_view->path(), true).path());
            dialog->set_exec_button_label(_("Move"));
            dialog->set_title(_("Move"));

            add_window_task(window, src, dialog->run(), [=] {
                return make_move_task(src->dir_vfs()->directory_type(),
                                      entries,
                                      expand_dest_path(src->path(), dialog->dest_path()));
            });
        }
    }
}


//// Delete Command Implementation

void delete_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    if (window && src) {
        auto entries = src->selected_entries();

        if (!entries.empty()) {
            // Query for confirmation

            Gtk::MessageDialog dialog(*window, _("Confirm Delete"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
            dialog.set_secondary_text(confirm_delete_msg(entries));

            add_window_task(window, src, dialog.run(), [=] {
                return make_delete_task(src->dir_vfs()->directory_type(), entries);
            });
        }
    }
}

Glib::ustring delete_command::confirm_delete_msg(const std::vector<dir_entry *> &ents) {
    if (ents.size() == 1) {
        return Glib::ustring::compose(_("Are you sure you want to delete '%1'?"), ents[0]->file_name());
    }
    else {
        return Glib::ustring::compose(_("Are you sure you want to delete %1 selected files?"), ents.size());
    }
}


//// Change Path Command Implementation

void change_path_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    if (src) {
        src->focus_path();
    }
}


//// Filter Command Implementations

void begin_filter_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    if (src) {
        src->begin_filter();
    }
}

void begin_filter_type_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase) {
    if (src) {
        if (e->type == GDK_KEY_PRESS) {
            GdkEventKey *key_event = (GdkEventKey*)e;
            gunichar chr = gdk_keyval_to_unicode(key_event->keyval);

            if (Glib::Unicode::isprint(chr))
                src->begin_filter(Glib::ustring(1, chr));
        }
        else {
            src->begin_filter();
        }
    }
}


//// Open Preferences Command Implementation

void preferences_command::run(nuc::app_window *, nuc::file_view *, const GdkEventAny *, Glib::VariantBase) {
    NuCommander::preferences();
}


//// Swap Panes Command Implementation

void swap_panes_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    nuc::file_view *dest = src->next_file_view;

    auto fl_src = src->file_list();
    auto fl_dest = dest->file_list();

    src->file_list(nullptr);
    dest->file_list(nullptr);

    src->file_list(fl_dest);
    dest->file_list(fl_src);
}


//// Change Directory Command Implementation

void change_dir_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    auto *popup = window->open_dirs_popup();

    popup->dir_chosen([=] (std::shared_ptr<file_list_controller> flist) {
        src->file_list(flist);
    });

    popup->show();
    popup->present();
}


//// Open New Directory Command Implementation

void open_dir_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    auto flist = directory_buffers::instance().new_buffer();
    auto old_path = src->path();

    src->file_list(flist);
    src->path(old_path);
}


//// Close Directory Command Implementation

void close_dir_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *, Glib::VariantBase) {
    auto &buffers = directory_buffers::instance();
    std::shared_ptr<file_list_controller> flist;

    while ((flist = src->pop_file_list()) && flist->attached());

    if (!flist) {
        auto it = buffers.buffers().begin(), end = buffers.buffers().end();

        while (it != end) {
            auto fl = *it;

            if (!fl->attached()) {
                flist = fl;
                break;
            }
            ++it;
        }
    }

    if (flist) {
        auto old_flist = src->file_list();

        src->file_list(flist, false);

        directory_buffers::instance().close_buffer(old_flist);
    }
}


//// Cancel Command Implementation

void cancel_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase) {
    if (src) {
        src->dir_vfs()->cancel();
    }
}


//// Quit Command Implementation

void quit_command::run(nuc::app_window *window, nuc::file_view *src, const GdkEventAny *e, Glib::VariantBase) {
    NuCommander::instance()->quit();
}
