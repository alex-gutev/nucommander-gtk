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

#include "settings/app_settings.h"

#include "copy/copy.h"

using namespace nuc;

/**
 * Copy command function.
 *
 * Copies the selected/marked files in the source pane, @a src, to the
 * destination pane's directory.
 *
 * @param window Pointer to the app_window, in which the command was
 *   executed.
 *
 * @param src Pointer to the source pane (file_vie), in which the
 *   command was executed.
 */
static void copy_command_fn(nuc::app_window *window, nuc::file_view *src);


// Initialize Builtin command table.

std::unordered_map<std::string, nuc::command_fn> nuc::commands{
    std::make_pair("copy", copy_command_fn)
};


/// Builtin Command Implementations

void copy_command_fn(nuc::app_window *window, nuc::file_view *src) {
    if (window && src)
        window->add_operation(src->make_copy_task(src->next_file_view->path()));
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