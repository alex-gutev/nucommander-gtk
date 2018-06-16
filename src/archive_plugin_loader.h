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
        
        /**
         * Stores the plugin details and a pointer to the
         * archive_plugin object.
         */
        struct plugin {
            /**
             * Path to the plugin shared library file.
             */
            std::string path;

            /**
             * Pointer to the archive_plugin object.
             */
            archive_plugin *plg;

            /**
             * Boolean flag indicating whether the plugin is loaded.
             */
            bool loaded = false;

            /**
             * Plugin constructor.
             *
             * Only intializes the fields however does not load the
             * plugin. To load the plugin the load method has to be
             * called.
             */
            plugin(std::string path) : path(std::move(path)), plg(new archive_plugin()) {}

            /**
             * Unloads the plugin and deletes the existing
             * archive_plugin object if any.
             */
            void unload() {
                if (plg) delete plg;
            }

            /**
             * Destructor, unloads the plugin.
             */
            ~plugin() {
                unload();
            }

            /**
             * Copy constructor deleted as there should only be a
             * single instances of an archive_plugin object, per
             * plugin, which is managed by this object.
             */
            plugin(const plugin&) = delete;
            plugin(plugin &&plg_obj) {
                plg = plg_obj.plg;
                plg_obj.plg = nullptr;
            }

            /**
             * Copy assignment operator deleted for the same reason as
             * the copy constructor.
             */
            plugin &operator=(const plugin &) = delete;
            plugin &operator=(plugin &&plg_obj) {
                unload();
                
                plg = plg_obj.plg;
                plg_obj.plg = nullptr;
            }

            /**
             * Loads the plugin and returns the pointer to the
             * archive_plugin object. If the plugin has already be
             * loaded the pointer is simply returned.
             *
             * @return Pointer to the archive_plugin object.
             */
            archive_plugin *load() {
                if (!loaded) {
                    loaded = true;
                    plg->load(path);                 
                }

                return plg;
            }
        };

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
         * Array of plugins
         */
        std::vector<plugin> plugins;
       

        /**
         * Constructor is private, as the class is meant to be used as
         * a singleton.
         */
        archive_plugin_loader();

        archive_plugin_loader(archive_plugin_loader &) = delete;
        archive_plugin_loader &operator=(archive_plugin_loader &) = delete;

        /**
         * Retrieves the plugin details from GSettings, and builds the
         * plugin regular expression.
         */
        void get_plugin_details();
        
    public:

        /**
         * Returns a reference to the singleton instance.
         */
        static archive_plugin_loader &instance();

        /**
         * Loads the archive plugin for the file at path @a path. If
         * the file name component of the path matches a particular
         * plugin's regular expression, the plugin is loaded and a
         * pointer to the archive_plugin instance is returned. If it
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
