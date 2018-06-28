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
#include <mutex>
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
         * ID of the GSettings schema of the application's settings.
         */
        static constexpr const char *settings_id = "org.agware.NuCommander";

        /**
         * GSettings path where the plugins' settings are stored.
         */
        static constexpr const char *settings_path = "/org/agware/NuCommander/";

        /**
         * ID of the plugin GSettings schema.
         */
        static constexpr const char *plugin_schema = "org.agware.NuCommander.plugin";

    public:

        /**
         * Responsible for loading and unloading the plugin.
         */
        struct plugin {
            /**
             * Path to the plugin shared library file.
             */
            std::string path;

            /**
             * Pointer to the archive_plugin object.
             */
            archive_plugin *plg = nullptr;

            /**
             * Mutex for synchronizing loading and unloading.
             */
            std::mutex mutex;

        public:

            /**
             * Plugin constructor.
             *
             * Only intializes the fields however does not load the
             * plugin. To load the plugin the load method has to be
             * called.
             */
            plugin(std::string path) : path(std::move(path)) {}

            /**
             * Destructor, unloads the plugin.
             */
            ~plugin() {
                std::lock_guard<std::mutex> lock(mutex);

                if (plg) {
                    delete plg;
                    plg = nullptr;
                }
            }

            /**
             * Copy constructor and assignment operation deleted as
             * there should only be a single instance per plugin.
             */
            plugin(const plugin&) = delete;
            plugin &operator=(const plugin &) = delete;

            /**
             * Loads the plugin and returns the pointer to the
             * archive_plugin object. If the plugin has already be
             * loaded the pointer is simply returned.
             *
             * @return Pointer to the archive_plugin object.
             */
            archive_plugin *load() {
                std::lock_guard<std::mutex> lock(mutex);

                if (!plg) {
                    plg = new archive_plugin();
                    plg->load(path);

                    path.clear();
                }

                return plg;
            }
        };

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
         * pointer to the plugin object is returned. If it doesn't
         * match any regular expression, nullptr is returned
         * indicating there is no plugin.
         *
         * @param path The file path.
         */
        plugin *get_plugin(const std::string &path);

    private:
        /**
         * GSettings object.
         */
        Glib::RefPtr<Gio::Settings> settings;

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
        std::vector<std::unique_ptr<plugin>> plugins;
       

        /**
         * Retrieves the plugin details from GSettings, and builds the
         * plugin regular expression.
         */
        void get_plugin_details();
    };
}

#endif // NUC_ARCHIVE_PLUGIN_LOADER

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
