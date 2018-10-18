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

#ifndef NUC_COMMANDS_H
#define NUC_COMMANDS_H

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

#include <giomm/settings.h>
#include <gdk/gdk.h>

namespace nuc {
    class app_window;
    class file_view;

    /**
     * Command function type.
     *
     * Takes two parameters:
     *
     *   - A pointer to the 'app_window' on which the command was run.
     *
     *   - A pointer to the source 'file_view' (if the command was run
     *     while the view is in focus).
     */
    typedef std::function<void(app_window *, file_view *)> command_fn;

    /**
     * Global command table.
     *
     * Maps command identifier names to the corresponding command
     * functions.
     */
    extern std::unordered_map<std::string, command_fn> commands;

    /**
     * Stores the command keymap which is retrieved from GSettings.
     */
    class command_keymap {
        /**
         * ID of the GSettings schema of the application's settings.
         */
        static constexpr const char *settings_id = "org.agware.NuCommander";

        /**
         * GSettings object.
         */
        Glib::RefPtr<Gio::Settings> settings;

        /**
         * The keymap.
         *
         * Each key is a string corresponding to a key sequence and
         * the value is the command bound to that key sequence.
         */
        std::unordered_map<std::string, std::string> keymap;

        /**
         * Retrieves the key map from GSettings.
         */
        void get_keymap();

        /**
         * Returns the key sequence string corresponding to a key
         * event.
         *
         * @param e The key event.
         *
         * @return The corresponding key sequence string or the empty
         *    string if there is no key sequence string for the event.
         */
        static std::string event_keystring(const GdkEventKey *e);

    public:

        /** Constructor */
        command_keymap();

        /**
         * Returns the singleton instance.
         */
        static command_keymap &instance();


        /**
         * Retrieves the name of the command bound to a particular key
         * sequence.
         *
         * @param key The key sequence string.
         *
         * @return The name of the command bound to the key sequence
         *   or the empty string if there is no command bound to the
         *   key sequence.
         */
        std::string command_name(const std::string &key);

        /**
         * Retrieves the name of the command that should be executed
         * in response to a key event.
         *
         * @param event The key event.
         *
         * @return The name of the command or the empty string if
         *   there no command should be executed.
         */
        std::string command_name(const GdkEventKey *event);
    };
}

#endif

// Local Variables:
// mode: c++
// End:
