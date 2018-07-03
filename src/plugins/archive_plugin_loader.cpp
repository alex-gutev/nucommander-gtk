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

#include "archive_plugin_loader.h"

using namespace nuc;

archive_plugin_loader::archive_plugin_loader()
    : settings(Gio::Settings::create(settings_id)) {
    get_plugin_details();
}

archive_plugin_loader &archive_plugin_loader::instance() {
    static archive_plugin_loader inst;

    return inst;
}

void archive_plugin_loader::get_plugin_details() {
    std::string reg;
    bool first = true;

    for (const auto &child : settings->get_string_array("plugins")) {
        Glib::ustring child_path(settings_path);

        child_path.append(child);
        child_path.append("/");

        Glib::RefPtr<Gio::Settings> plg = Gio::Settings::create(plugin_schema, child_path);

        std::string path = plg->get_string("path");
        std::string ext_reg = plg->get_string("regex");

        plugins.emplace_back(new archive_plugin(path));

        if (!first) {
            reg.push_back('|');
        }

        reg.push_back('(');
        reg.append(ext_reg);
        reg.push_back(')');

        first = false;
    }

    regex.assign(reg);
}

archive_plugin *archive_plugin_loader::get_plugin(const std::string &path) {
    std::smatch results;

    if (std::regex_match(path, results, regex)) {
        // Find first capture group which matched
        for (size_t i = 1; i < results.size(); i++) {
            if (results.length(i)) {
                return plugins[i - 1].get();
            }
        }
    }

    return nullptr;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
