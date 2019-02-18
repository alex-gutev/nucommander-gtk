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

#ifndef NUC_COMMANDS_LUA_API_H
#define NUC_COMMANDS_LUA_API_H

#include <lua.hpp>

namespace nuc {
    class app_window;
    class file_view;

    /**
     * Register the NuCommander Lua API with the Lua interpreter.
     *
     * @param L Lua interpreter state.
     */
    void register_nuc_api(lua_State *L);

    /**
     * Pass the window and source pane command arguments to a Lua
     * script.
     *
     * Sets the global 'window' variable to @a window and the global
     * 'source_pane' variable to @a src.
     *
     * @param L Lua interpreter state.
     * @param window The app_window.
     * @param src The source file_view.
     */
    void pass_lua_command_args(lua_State *L, app_window *window, file_view *src);
}  // nuc

#endif /* NUC_COMMANDS_LUA_API_H */

// Local Variables:
// mode: c++
// End:
