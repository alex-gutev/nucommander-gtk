/*
 * NuCommander
 * Copyright (C) 2018  Alexander Gutev <alex.gutev@gmail.com>
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

#include "commands.h"

#include <gtkmm/messagedialog.h>

#include "app_window.h"
#include "file_view.h"

#include "interface/key_prefs_window.h"
#include "interface/dest_dialog.h"

#include "settings/app_settings.h"

#include "operations/copy.h"
#include "operations/move.h"
#include "operations/delete.h"

#include "file_list/directory_buffers.h"

using namespace nuc;

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

/**
 * Copy command function.
 *
 * Queries the user for a destination directory, via the destination
 * dialog, and copies the selected/marked files in the source pane, @a
 * src, to that directory.
 *
 * @param window Pointer to the app_window, in which the command was
 *  triggered.
 *
 * @param src Pointer to the source pane (file_view), in which the
 *   command was triggered.
 */
static void copy_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

/**
 * Make directory command function.
 *
 * Displays a destination path dialog which queries the user for the
 * name of the directory to create.
 *
 * Adds a task to the window's task queue which creates the directory
 * in the source path.
 *
 * @param window Pointer to the app_window, in which the command was
 *   triggered.
 *
 * @param src Pointer to the source pane (file_view), in which the
 *   command was triggered.
 */
static void make_dir_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

/**
 * Make directory task function. Creates the directory @a name in the
 * directory @a dest.
 *
 * @param state Task cancellation state.
 * @param dest The directory in which to create the directory.
 * @param name Name of the directory to create.
 */
static void make_dir_task(nuc::cancel_state &state, const nuc::paths::string &dest, const nuc::paths::string &name);

/**
 * Move command function.
 *
 * Queries the user for a destination directory, via the destination
 * dialog, and moves the selected/marked files in the source pane, @a
 * src, to the that directory.
 *
 * @param window Pointer to the app_window, in which the command was
 *   triggered.
 *
 * @param src Pointer to the source pane (file_view), in which the
 *   command was triggered.
 */
static void move_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

/**
 * Delete command function.
 *
 * Deletes the selected files in the source pane, after querying the
 * user for confirmation.
 *
 * @param window Pointer to the app_window, in which the command was
 *   triggered.
 *
 * @param src Pointer to the source pane (file_view), in which the
 *   command was triggered.
 */
static void delete_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

/**
 * Creates the delete confirmation message.
 *
 * @param ents Array of selected entries to be deleted.
 *
 * @return The confirmation message.
 */
static Glib::ustring confirm_delete_msg(const std::vector<dir_entry *> &ents);

/**
 * Change path command function.
 *
 * Moves the keyboard focus to the current path entry.
 *
 * @param window Pointer to the app_window, in which the command was
 *   triggered.
 *
 * @param src Pointer to the source pane (file_view), in which the
 *   command was triggered.
 */
static void jump_path_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

/**
 * Open key binding preferences command function.
 */
static void open_key_prefs_command_fn(nuc::app_window *, nuc::file_view *, Glib::VariantBase);

/**
 * Swap panes command function.
 *
 * Swaps the left and right file list panes.
 *
 * @param window Pointer to the window in which the command was
 *   triggered.
 *
 * @param src Pointer to the source pane (file_view), in which the
 *   command was triggered.
 */
static void swap_panes_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

/**
 * Change directory command function.
 *
 * Displays the open directory list popup.
 *
 * @param window Pointer to the window in which the command was
 *   triggered.
 *
 * @param src Pointer to the source pane (file_view), in which the
 *   command was triggered.
 */
static void change_dir_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

/**
 * Create new open directory command function.
 *
 * Creates a new file_list_controller and sets it as the
 * file_list_controller of the source pane.
 *
 * @param window Pointer to the window in which the command was
 *   triggered.
 *
 * @param src Pointer to the source pane (file_view), in which the
 *   command was triggered.
 */
static void open_dir_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);

/**
 * Close directory buffer command.
 *
 * Deletes the file_list_controller of the source pane and switches to
 * the file_list_controller in the stack.
 *
 * @details    detailed description
 *
 * @param window Pointer to the window in which the command was
 *   triggered.
 *
 * @param src Pointer to the source pane (file_view), in which the
 *   command was triggered.
 */
static void close_dir_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase);


// Initialize Builtin command table.

std::unordered_map<std::string, nuc::command_fn> nuc::commands{
    std::make_pair("copy", copy_command_fn),
    std::make_pair("make-directory", make_dir_command_fn),
    std::make_pair("move", move_command_fn),
    std::make_pair("delete", delete_command_fn),
    std::make_pair("jump-path", jump_path_command_fn),
    std::make_pair("open-key-prefs", open_key_prefs_command_fn),
    std::make_pair("swap-panes", swap_panes_command_fn),
    std::make_pair("change-directory", change_dir_command_fn),
    std::make_pair("open-new-directory", open_dir_command_fn),
    std::make_pair("close-directory", close_dir_command_fn)
};


paths::pathname expand_dest_path(const paths::pathname &path, const paths::pathname &dest) {
    return paths::pathname(path, true).merge(dest);
}


/// Builtin Command Implementations

void copy_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
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

void make_dir_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
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

void make_dir_task(cancel_state &state, const paths::string &dest, const paths::string &name) {
    try {
        std::unique_ptr<dir_writer> writer(dir_type::get_writer(dest));

        writer->mkdir(name, false);
        writer->close();
    }
    catch (const error &e) {
        // Catch error to abort operation
    }
}

void move_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
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

void delete_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
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

Glib::ustring confirm_delete_msg(const std::vector<dir_entry *> &ents) {
    if (ents.size() == 1) {
        return Glib::ustring::compose("Are you sure you want to delete '%1'?", ents[0]->file_name());
    }
    else {
        return Glib::ustring::compose("Are you sure you want to delete %1 selected files?", ents.size());
    }
}


void jump_path_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    if (src) {
        src->focus_path();
    }
}

void open_key_prefs_command_fn(nuc::app_window *, nuc::file_view *, Glib::VariantBase) {
    auto *window = key_prefs_window::instance();

    window->show();
    window->present();
}


void swap_panes_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    nuc::file_view *dest = src->next_file_view;

    auto fl_src = src->file_list();
    auto fl_dest = dest->file_list();

    src->file_list(nullptr);
    dest->file_list(nullptr);

    src->file_list(fl_dest);
    dest->file_list(fl_src);
}

void change_dir_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    auto *popup = window->open_dirs_popup();

    popup->dir_chosen([=] (std::shared_ptr<file_list_controller> flist) {
        src->file_list(flist);
    });

    popup->show();
    popup->present();
}

void open_dir_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
    auto flist = directory_buffers::instance().new_buffer();
    auto old_path = src->file_list()->path();

    src->file_list(flist);
    src->path(old_path);
}

static void close_dir_command_fn(nuc::app_window *window, nuc::file_view *src, Glib::VariantBase) {
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


/// command_keymap Implementation

command_keymap::command_keymap() {
    get_keymap();

    app_settings::instance().settings()->signal_changed("keybindings").connect(sigc::mem_fun(this, &command_keymap::keymap_changed));
};

command_keymap &command_keymap::instance() {
    static command_keymap inst;

    return inst;
}

void command_keymap::get_keymap() {
    Glib::Variant<std::map<Glib::ustring, Glib::ustring>> gv_map;
    app_settings::instance().settings()->get_value("keybindings", gv_map);

    auto map = gv_map.get();

    keymap = std::unordered_map<std::string, std::string>(map.begin(), map.end());
}

void command_keymap::keymap_changed(const Glib::ustring &key) {
    get_keymap();
}


std::string command_keymap::command_name(const std::string &key) {
    auto command = keymap.find(key);

    if (command != keymap.end())
        return command->second;

    return "";
}

std::string command_keymap::command_name(const GdkEventKey *e) {
    return command_name(event_keystring(e));
}

std::string command_keymap::event_keystring(const GdkEventKey *e) {
    auto code = e->keyval;

    std::string keystring;

    if (e->state & GDK_CONTROL_MASK) {
        keystring.append("C-");
    }
    if (e->state & GDK_MOD1_MASK) {
        keystring.append("M-");
    }
    if (e->state & GDK_SHIFT_MASK) {
        keystring.append("S-");
    }

    switch (code) {
    case GDK_KEY_Return:
        keystring.append("Return");
        break;

    case GDK_KEY_Tab:
        keystring.append("Tab");
        break;

    case GDK_KEY_BackSpace:
        keystring.append("Backspace");
        break;

    case GDK_KEY_Escape:
        keystring.append("Escape");
        break;

    case GDK_KEY_Delete:
        keystring.append("Delete");
        break;

    case GDK_KEY_space:
        keystring.append("Space");
        break;

    default:
        if (gunichar chr = gdk_keyval_to_unicode(code)) {
            keystring.append(Glib::ustring(1, chr));
        }
        else if (gchar *name = gdk_keyval_name(code)) {
            keystring.append(name);
        }
        else {
            return "";
        }
        break;
    }

    return keystring;
}

bool command_keymap::exec_command(app_window *window, file_view *src, const GdkEventKey *e, Glib::VariantBase arg) {
    auto command = commands.find(command_name(e));

    if (command != commands.end()) {
        command->second(window, src, arg);
        return true;
    }

    return false;
}
