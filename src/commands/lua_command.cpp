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

#include "app_window.h"
#include "file_view.h"

#include "luawrapper/luawrapper.hpp"

using namespace nuc;

/**
 * NucWindow (app_window wrapper) table and metatable.
 */
static luaL_Reg NucWindow_table[] =
{
    {NULL, NULL}
};
static luaL_Reg NucWindow_metatable[] =
{
    {NULL, NULL}
};

/**
 * NucPane (file_view wrapper) table and metatable.
 */
static luaL_Reg NucPane_table[] =
{
    {NULL, NULL}
};
static luaL_Reg NucPane_metatable[] =
{
    {NULL, NULL}
};

/**
 * Execute Command Lua Function.
 *
 * Called from Lua code to execute a command by name.
 */
static int exec_command(lua_State *L);

/**
 * Registers the C/C++ functions and classes for use in Lua code.
 *
 * @param L Lua interpreter state.
 */
static void register_lua_funcs(lua_State *L);


void lua_command::init_state() {
    if (!state) {
        if (!(state = luaL_newstate())) {
            throw error("Error allocating Lua interpreter state.");
        }

        luaL_openlibs(state);

        lua_register(state, "exec_command", exec_command);
        register_lua_funcs(state);

        if (luaL_loadfile(state, path.c_str())) {
            raise_lua_error();
        }

        lua_setglobal(state, "command_fn");
    }
}

std::string lua_command::get_error_desc() {
    std::string err = luaL_checkstring(state, -1);
    lua_pop(state, 1);

    return err;
}

void lua_command::raise_lua_error() {
    throw error(get_error_desc());
}


lua_command::~lua_command() {
    if (state) lua_close(state);
}

void lua_command::run(app_window *window, file_view *src, Glib::VariantBase arg) {
    init_state();

    lua_createtable(state, 2, 0);

    lua_pushnumber(state, 1);
    luaW_push<app_window>(state, window);
    lua_settable(state, -3);

    lua_pushnumber(state, 2);
    luaW_push<file_view>(state, src);
    lua_settable(state, -3);

    lua_setglobal(state, "arg");

    lua_getglobal(state, "command_fn");
    if (lua_pcall(state, 0, 0, 0)) {
        raise_lua_error();
    }
}


void register_lua_funcs(lua_State *L) {
    luaW_register<app_window>(L, "NucWindow", NucWindow_table, NucWindow_metatable, NULL, NULL);
    luaW_register<file_view>(L, "NucPane", NucPane_table, NucPane_metatable, NULL, NULL);
}

int exec_command(lua_State *L) {
    // Command, window, pane
    int n = lua_gettop(L);

    if (n == 3) {
        const char *cmd = luaL_checkstring(L, 1);

        // May throw a Lua exception so be careful,
        // use luaW_to return NULL if the type is incorrect.
        app_window *window = luaW_check<app_window>(L, 2);
        file_view *view = luaW_check<file_view>(L, 3);

        command_keymap::instance().exec_command(cmd, window, view);
    }

    // TODO: Signal error otherwise

    return 0;
}
