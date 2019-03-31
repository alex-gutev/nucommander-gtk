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

#include "settings/app_settings.h"

#include "builtin.h"
#include "custom_commands.h"

#include "tasks/async_task.h"

using namespace nuc;

/**
 * Returns a generic keystring for the key event @a e which represents
 * the type of key pressed, e.g. <char> is returned for keys which
 * produce a printable character.
 *
 * @param e The key event.
 *
 * @return The keystring, empty if there is no generic keystring.
 */
static std::string generic_keystring(const GdkEventKey *e) {
    return gdk_keyval_to_unicode(e->keyval) > 0x20 ? "<char>" : "";
}

/// command_keymap Implementation

command_keymap::command_keymap() {
    // Initially make builtin commands available
    add_builtin_commands(command_table);

    // Load custom commands in background so as to increase
    // application load time.
    load_custom_commands();

    get_keymap();

    app_settings::instance().settings()->signal_changed("keybindings").connect(sigc::mem_fun(this, &command_keymap::keymap_changed));
};

command_keymap &command_keymap::instance() {
    static command_keymap inst;

    return inst;
}


void command_keymap::load_custom_commands() {
    dispatch_async([=] {
        command_map table;

        add_builtin_commands(table);
        add_custom_commands(table);

        set_command_map(table);
    });
}

void command_keymap::set_command_map(command_map &map) {
    dispatch_main([=] {
        command_table = std::move(map);
    });
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
    auto cmd = command_name(event_keystring(e));

    return cmd.length() ? cmd : command_name(generic_keystring(e));
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
    return exec_command(command_name(e), window, src, (const GdkEventAny *)e, arg);
}

bool command_keymap::exec_command(const std::string &name, app_window *window, file_view *src, const GdkEventAny *e, Glib::VariantBase arg) {
    auto command = command_table.find(name);

    if (command != command_table.end()) {
        command->second->run(window, src, e, arg);
        return true;
    }

    return false;
}
