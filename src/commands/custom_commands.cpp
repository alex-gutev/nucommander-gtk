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

#include "util/util.h"

#include "settings/app_settings.h"
#include "commands/lua_command.h"

using namespace nuc;

#include <iostream>
void nuc::add_custom_commands(std::unordered_map<std::string, std::unique_ptr<command>> &table) {
    auto settings = app_settings::instance().settings();
    Glib::VariantContainerBase commands;

    settings->get_value("custom-commands", commands);

    for (size_t i = 0; i < commands.get_n_children(); ++i) {
        // Command Dictionary Entry
        Glib::VariantContainerBase entry;

        commands.get_child(entry, i);

        Glib::Variant<std::string> name; // Dictionary Key
        Glib::VariantContainerBase command; // Dictionary Value

        Glib::Variant<std::string> path;
        Glib::Variant<std::string> desc;

        entry.get_child(name, 0);
        entry.get_child(command, 1);

        command.get_child(desc, 0);
        command.get_child(path, 1);

        table.emplace(name.get(), make_unique(new lua_command(path.get(), desc.get())));
    }
}
