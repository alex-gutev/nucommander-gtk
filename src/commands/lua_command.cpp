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

using namespace nuc;

void lua_command::init_state() {
    if (!state) {
        if (!(state = luaL_newstate())) {
            throw error();
        }

        luaL_openlibs(state);

        if (luaL_loadfile(state, path.c_str())) {
            throw error();
        }

        lua_setglobal(state, "command_fn");
    }
}

lua_command::~lua_command() {
    if (state) lua_close(state);
}

void lua_command::run(app_window *window, file_view *src, Glib::VariantBase arg) {
    init_state();

    lua_getglobal(state, "command_fn");

    if (lua_pcall(state, 0, 0, 0)) {
        throw error();
    }
}
