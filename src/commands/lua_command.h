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

#ifndef NUC_COMMANDS_LUA_COMMAND_H
#define NUC_COMMANDS_LUA_COMMAND_H

#include <utility>
#include <exception>

#include <lua.hpp>

#include "errors/errors.h"

#include "commands.h"

namespace nuc {
    /**
     * Custom command implemented in a Lua script.
     */
    class lua_command : public command {
    public:
        /**
         * Lua Error Exception.
         */
        class error : public nuc::error {
        public:
            template <typename T>
            error(T&& desc) : nuc::error(-1, type_general, true, std::forward<T>(desc)) {}
        };

        /**
         * Create a Lua command functor with a given path and
         * description.
         *
         * @param path Path to the Lua script file.
         * @param desc Command description.
         */
        lua_command(std::string path, std::string desc = "") : desc(std::move(desc)), path(std::move(path)) {}

        virtual ~lua_command();

        virtual void run(app_window *window, file_view *src, const GdkEventAny *e, Glib::VariantBase arg);

        virtual std::string description() {
            return desc;
        }

    private:
        /**
         * Lua Interpreter State.
         */
        lua_State *state = nullptr;

        /**
         * Command Description.
         */
        std::string desc;
        /**
         * Path to the Lua script.
         */
        std::string path;

        /**
         * Initialize the Lua interpreter state if not already
         * initialized.
         */
        void init_state();


        /**
         * Retrieve the last error description string from the Lua
         * stack.
         *
         * @return The description string.
         */
        std::string get_error_desc();

        /**
         * Throw an error exception with the description string
         * retrieved from the Lua stack.
         */
        void raise_lua_error();
    };
}  // nuc

#endif /* NUC_COMMANDS_LUA_COMMAND_H */

// Local Variables:
// mode: c++
// End:
