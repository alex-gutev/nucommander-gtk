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

#include "lua_command.h"

#include <giomm/appinfo.h>

#include "app_window.h"
#include "file_view.h"

#include "lua_api.h"

using namespace nuc;

/**
 * The registry key under which the command chunk is stored.
 */
static constexpr const char *nuc_command_key = "org.agware.nucommander.command";


//// Initialization

void lua_command::init_state() {
    if (!state) {
        if (!(state = luaL_newstate())) {
            throw error("Error allocating Lua interpreter state.");
        }

        luaL_openlibs(state);
        register_nuc_api(state);

        if (luaL_loadfile(state, path.c_str())) {
            raise_lua_error();
        }

        lua_setfield(state, LUA_REGISTRYINDEX, nuc_command_key);
    }
}


//// Error Handling

std::string lua_command::get_error_desc() {
    std::string err = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    return err;
}

void lua_command::raise_lua_error() {
    throw error(get_error_desc());
}


//// Cleanup

lua_command::~lua_command() {
    if (state) lua_close(state);
}


//// Script Execution

void lua_command::run(app_window *window, file_view *src, Glib::VariantBase arg) {
    init_state();

    pass_lua_command_args(state, window, src);

    lua_getfield(state, LUA_REGISTRYINDEX, nuc_command_key);
    if (lua_pcall(state, 0, 0, 0)) {
        raise_lua_error();
    }
}
