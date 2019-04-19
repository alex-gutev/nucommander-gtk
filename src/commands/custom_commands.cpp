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

#include "custom_commands.h"

#include <map>

#include <glibmm/miscutils.h>
#include <boost/algorithm/string.hpp>

#include "util/util.h"
#include "lister/dir_lister.h"
#include "commands/lua_command.h"


using namespace nuc;

/**
 * Return the list of directories from which to load commands.
 *
 * @return Vector of pathname's of the directories.
 */
static std::vector<pathname> command_dirs();

/**
 * Loads all custom commands in the directory @a dir to the command
 * table @a table.
 *
 * @param dir Directory to load commands from.
 * @param table Command table to load the commands to.
 */
static void load_commands_in_dir(const pathname &dir, command_keymap::command_map &table);


void nuc::add_custom_commands(command_keymap::command_map &table) {
    for (auto &path : command_dirs()) {
        try {
            load_commands_in_dir(path, table);
        }
        catch (const nuc::error &e) {
            // Skip loading commands from directory
        }
    }
}

static std::vector<pathname> command_dirs() {
    std::vector<pathname> dirs;

    for (const auto &path : Glib::get_system_data_dirs()) {
        dirs.push_back(pathname(path).append("nucommander/commands"));
    }

    dirs.push_back(pathname(Glib::get_user_data_dir()).append("nucommander/commands"));

    return dirs;
}

static void load_commands_in_dir(const pathname &dir, command_keymap::command_map &table) {
    dir_lister listr(dir);

    lister::entry ent;
    struct stat st;

    while (listr.read_entry(ent)) {
        if (ent.type != DT_REG) {
            if (!listr.entry_stat(st) || S_ISREG(st.st_mode))
                continue;
        }

        pathname name(ent.name);

        if (boost::iequals(name.extension(), "LUA")) {
            const std::string &name_str = name.path();
            table[name_str.substr(0, name_str.length() - 4)] = make_unique(new lua_command(dir.append(name)));
        }
    }
}
