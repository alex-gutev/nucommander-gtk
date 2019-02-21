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

#include "app_window.h"
#include "file_view.h"

#include "interface/key_prefs_window.h"
#include "interface/dest_dialog.h"

#include "operations/copy.h"
#include "operations/move.h"
#include "operations/delete.h"

#include "file_list/directory_buffers.h"

#include "util/util.h"

using namespace nuc;


//// Prototypes

/// Utility Functions

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
static paths::pathname expand_dest_path(const paths::pathname &path, const paths::pathname &dest);


/// Builtin Commands

/**
 * Copy Command.
 *
 * Queries the user for a destination directory, via the destination
 * dialog, and copies the selected/marked files in the source pane to
 * the directory.
 */
struct copy_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

    virtual std::string description() {
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
    virtual void run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

    /**
     * Make directory task function. Creates the directory @a name in the
     * directory @a dest.
     *
     * @param state Task cancellation state.
     * @param dest The directory in which to create the directory.
     * @param name Name of the directory to create.
     */
    static void make_dir_task(nuc::cancel_state &state, const nuc::paths::string &dest, const nuc::paths::string &name);

    virtual std::string description() {
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
    virtual void run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

    virtual std::string description() {
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
    virtual void run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

    /**
     * Creates the delete confirmation message.
     *
     * @param ents Array of selected entries to be deleted.
     *
     * @return The confirmation message.
     */
    static Glib::ustring confirm_delete_msg(const std::vector<dir_entry *> &ents);

    virtual std::string description() {
        return _("Delete the marked/selected files in the source pane");
    }
};

/**
 * Change Path Command.
 *
 * Moves the keyboard focus to the current path entry.
 */
struct jump_path_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

    virtual std::string description() {
        return _("Jump to the current path text entry.");
    }
};

/**
 * Open Key Binding Preferences Command.
 */
struct open_key_prefs_command : public command {
    virtual void run(nuc::app_window *, nuc::file_view *, Glib::VariantBase);

    virtual std::string description() {
        return _("Open the keybinding preferences.");
    }
};

/**
 * Swap Panes Command.
 *
 * Swaps the left and right file list panes.
 */
struct swap_panes_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

    virtual std::string description() {
        return _("Swap source and destination panes' directories.");
    }
};

/**
 * Change Directory Command.
 *
 * Displays the open directory list popup.
 */
struct change_dir_command : public command {
    virtual void run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

    virtual std::string description() {
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
    virtual void run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

    virtual std::string description() {
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
    virtual void run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

    virtual std::string description() {
        return _("Close the source pane's current directory and return to the previously visited directory.");
    }
};


//// Implementations

void nuc::add_builtin_commands(std::unordered_map<std::string, std::unique_ptr<command>> &table) {
    table.emplace("copy", make_unique(new copy_command));
    table.emplace("make-directory", make_unique(new make_dir_command));
    table.emplace("move", make_unique(new move_command));
    table.emplace("delete", make_unique(new delete_command));
    table.emplace("jump-path", make_unique(new jump_path_command));
    table.emplace("open-key-prefs", make_unique(new open_key_prefs_command));
    table.emplace("swap-panes", make_unique(new swap_panes_command));
    table.emplace("change-directory", make_unique(new change_dir_command));
    table.emplace("open-new-directory", make_unique(new open_dir_command));
    table.emplace("close-directory", make_unique(new close_dir_command));
}


/// Utility Functions

paths::pathname expand_dest_path(const paths::pathname &path, const paths::pathname &dest) {
    return paths::pathname(path, true).merge(dest);
}


/// Builtin Command Implementations

void copy_command::run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    if (window && src) {
        auto entries = src->file_list()->selected_entries();

        if (!entries.empty()) {
            dest_dialog *dialog = window->dest_dialog();

            dialog->set_query_label("Copy to:");
            dialog->dest_path(paths::pathname(src->next_file_view->path(), true).path());
            dialog->set_exec_button_label("Copy");

            if (dialog->run() == Gtk::RESPONSE_OK) {
                auto type = src->file_list()->dir_vfs()->directory_type();

                window->add_operation(
                    make_copy_task(type, entries, expand_dest_path(src->path(), dialog->dest_path())),
                    window->get_progress_fn(type));
            }
        }
    }
}

void make_dir_command::run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    using namespace std::placeholders;

    if (window && src) {
        paths::string dest = src->path();

        dest_dialog *dialog = window->dest_dialog();

        dialog->set_query_label("Directory Name:");
        dialog->dest_path("");
        dialog->set_exec_button_label("Create Directory");

        if (dialog->run() == Gtk::RESPONSE_OK) {
            window->add_operation(std::bind(make_dir_task, _1, dest, dialog->dest_path()));
        }
    }
}


void make_dir_command::make_dir_task(cancel_state &state, const paths::string &dest, const paths::string &name) {
    try {
        std::unique_ptr<dir_writer> writer(dir_type::get_writer(dest));

        writer->mkdir(name, false);
        writer->close();
    }
    catch (const error &e) {
        // Catch error to abort operation
    }
}


void move_command::run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    if (window && src) {
        auto entries = src->file_list()->selected_entries();

        if (!entries.empty()) {
            dest_dialog *dialog = window->dest_dialog();

            dialog->set_query_label("Move/Rename to:");
            dialog->dest_path(paths::pathname(src->next_file_view->path(), true).path());
            dialog->set_exec_button_label("Move");

            if (dialog->run() == Gtk::RESPONSE_OK) {
                auto type = src->file_list()->dir_vfs()->directory_type();

                window->add_operation(
                    make_move_task(type, entries, expand_dest_path(src->path(), dialog->dest_path())),
                    window->get_progress_fn(type));
            }
        }
    }
}


void delete_command::run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    if (window && src) {
        auto entries = src->file_list()->selected_entries();

        if (!entries.empty()) {
            // Query for confirmation

            Gtk::MessageDialog dialog(*window, "Confirm Delete", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
            dialog.set_secondary_text(confirm_delete_msg(entries));

            int result = dialog.run();

            // Add delete operation if response was OK

            if (result == Gtk::RESPONSE_OK) {
                auto type = src->file_list()->dir_vfs()->directory_type();

                window->add_operation(make_delete_task(type, entries),
                                      window->get_progress_fn(type));
            }
        }
    }
}

Glib::ustring delete_command::confirm_delete_msg(const std::vector<dir_entry *> &ents) {
    if (ents.size() == 1) {
        return Glib::ustring::compose("Are you sure you want to delete '%1'?", ents[0]->file_name());
    }
    else {
        return Glib::ustring::compose("Are you sure you want to delete %1 selected files?", ents.size());
    }
}


void jump_path_command::run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    if (src) {
        src->focus_path();
    }
}

void open_key_prefs_command::run(nuc::app_window *, nuc::file_view *, Glib::VariantBase) {
    auto *window = key_prefs_window::instance();

    window->show();
    window->present();
}

void swap_panes_command::run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    nuc::file_view *dest = src->next_file_view;

    auto fl_src = src->file_list();
    auto fl_dest = dest->file_list();

    src->file_list(nullptr);
    dest->file_list(nullptr);

    src->file_list(fl_dest);
    dest->file_list(fl_src);
}

void change_dir_command::run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    auto *popup = window->open_dirs_popup();

    popup->dir_chosen([=] (std::shared_ptr<file_list_controller> flist) {
        src->file_list(flist);
    });

    popup->show();
    popup->present();
}

void open_dir_command::run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    auto flist = directory_buffers::instance().new_buffer();
    auto old_path = src->file_list()->path();

    src->file_list(flist);
    src->path(old_path);
}

void close_dir_command::run(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
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
