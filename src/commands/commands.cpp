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

#include "app_window.h"
#include "file_view.h"

#include "interface/dest_dialog.h"

#include "settings/app_settings.h"

#include "copy/copy.h"
#include "copy/move.h"

using namespace nuc;

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
static void copy_command_fn(nuc::app_window *window, nuc::file_view *src);

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
static void make_dir_command_fn(nuc::app_window *window, nuc::file_view *src);

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
static void move_command_fn(nuc::app_window *window, nuc::file_view *src);

// Initialize Builtin command table.

std::unordered_map<std::string, nuc::command_fn> nuc::commands{
    std::make_pair("copy", copy_command_fn),
    std::make_pair("make-directory", make_dir_command_fn),
    std::make_pair("move", move_command_fn)
};


/// Builtin Command Implementations

void copy_command_fn(nuc::app_window *window, nuc::file_view *src) {
    if (window && src) {
        paths::string dest = src->next_file_view->path();

        dest_dialog *dialog = window->dest_dialog();

        dialog->set_query_label("Copy to:");
        dialog->set_dest_entry_text(dest);
        dialog->set_exec_button_label("Copy");

        window->dest_dialog()->show([=] (const Glib::ustring &path) {
            if (auto task = src->make_copy_task(path))
                window->add_operation(task);
        });
    }
}

void make_dir_command_fn(nuc::app_window *window, nuc::file_view *src) {
    using namespace std::placeholders;

    if (window && src) {
        paths::string dest = src->path();

        dest_dialog *dialog = window->dest_dialog();

        dialog->set_query_label("Directory Name:");
        dialog->set_dest_entry_text("");
        dialog->set_exec_button_label("Create Directory");

        window->dest_dialog()->show([=] (const Glib::ustring &name) {
            window->add_operation(std::bind(make_dir_task, _1, dest, name));
        });
    }
}

void make_dir_task(cancel_state &state, const paths::string &dest, const paths::string &name) {
    std::unique_ptr<dir_writer> writer(dir_type::get_writer(dest));

    writer->mkdir(name);
}

void move_command_fn(nuc::app_window *window, nuc::file_view *src) {
    if (window && src) {
        paths::string dest = src->next_file_view->path();

        dest_dialog *dialog = window->dest_dialog();

        dialog->set_query_label("Move/Rename to:");
        dialog->set_dest_entry_text(dest);
        dialog->set_exec_button_label("Move");

        window->dest_dialog()->show([=] (const Glib::ustring &path) {
            auto entries = src->file_list().selected_entries();

            if (!entries.empty()) {
                window->add_operation(make_move_task(src->file_list().dir_vfs()->directory_type(), entries, path));
            }
        });
    }
}


/// command_keymap Implementation

command_keymap::command_keymap() {
    get_keymap();
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
