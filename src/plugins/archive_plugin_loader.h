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

#ifndef NUC_ARCHIVE_PLUGIN_LOADER_H
#define NUC_ARCHIVE_PLUGIN_LOADER_H

#include <unordered_map>
#include <vector>
#include <regex>
#include <utility>
#include <memory>

#include <giomm/settings.h>

#include "archive_plugin.h"

namespace nuc {
    /**
     * Responsible for loading archive plugins, and provides
     * functionality for determining the correct plugin for the
     * archive file.
     */
    class archive_plugin_loader {
        /**
         * ID of the plugin GSettings schema.
         */
        static constexpr const char *plugin_schema = "org.agware.NuCommander.plugin";

        /**
         * Archive file name regular expression.
         *
         * This regular expression is an OR expression of all plugin
         * regular expressions. Each plugin regular expression is
         * enclosed in its own capture group, the index of which
         * corresponds to the index of the plugin within the plugins
         * array.
         */
        std::regex regex;

        /**
         * Plugins array.
         *
         * The index at which each plugin object is located corresponds
         * to the index of its capture group in the regular
         * expression.
         */
        std::vector<std::unique_ptr<archive_plugin>> plugins;


        /**
         * Retrieves the plugin details from GSettings, and builds the
         * plugin regular expression.
         */
        void get_plugin_details();

    public:

        /**
         * Constructor: Retrieves the plugin configuration details
         * from GSettings.
         */
        archive_plugin_loader();

        /**
         * Returns a reference to the singleton instance.
         */
        static archive_plugin_loader &instance();

        /**
         * Loads the archive plugin for the file at path @a path. If
         * the file name component of the path matches a particular
         * plugin's regular expression, the plugin is loaded and a
         * pointer to the archive_plugin object is returned. If it
         * doesn't match any regular expression, nullptr is returned
         * indicating there is no plugin.
         *
         * @param path The file path.
         */
        archive_plugin *get_plugin(const std::string &path);
    };
}

#endif // NUC_ARCHIVE_PLUGIN_LOADER

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
