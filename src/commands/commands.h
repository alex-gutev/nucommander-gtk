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

#include <glibmm/variant.h>
#include <giomm/settings.h>
#include <gdk/gdk.h>

namespace nuc {
    class app_window;
    class file_view;

    /**
     * Command base functor class.
     */
    struct command {
        /* Virtual destructor */
        virtual ~command() = default;

        /**
         * Execute the command.
         *
         * @param window The window in which the command was triggered.
         *
         * @param src The pane in which the command was triggered
         *   (i.e. the source pane).
         *
         * @param arg Optional command argument.
         */
        virtual void run(app_window *window, file_view *src, Glib::VariantBase arg) = 0;

        /**
         * Returns a description of the command.
         *
         * @return string.
         */
        virtual std::string description() = 0;
    };

    /**
     * Stores the command keymap which is retrieved from GSettings.
     */
    class command_keymap {
    public:
        /**
         * Global Command Table.
         *
         * Each key is a unique string identifying the command and the
         * corresponding value is the command functor.
         */
        std::unordered_map<std::string, std::unique_ptr<command>> command_table;


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

        /**
         * Executes the command which is bound to the key-sequence
         * corresponding to the key event @a event.
         *
         * @param window The window in which the event occurred.
         *
         * @param src The file_view in which the event occurred.
         *
         * @param event The event.
         *
         * @param arg Optional command argument.
         *
         * @return boolean True if a command was executed, false if
         *    there is no command bound to the key-sequence.
         */
        bool exec_command(app_window *window, file_view *src, const GdkEventKey *event, Glib::VariantBase arg = Glib::VariantBase());

        /**
         * Execute the named command @a name.
         *
         * @param window The window in which the command should be
         *   executed.
         *
         * @param src The source pane (file_view).
         *
         * @param arg Optional command argument.
         */
        bool exec_command(const std::string &name, app_window *window, file_view *src, Glib::VariantBase arg = Glib::VariantBase());

    private:
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
         * Default handler for the settings changed signal.
         *
         * Updates keymap.
         *
         * @param      key The changed key.
         */
        void keymap_changed(const Glib::ustring &key);

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
    };
}

#endif

// Local Variables:
// mode: c++
// End:
