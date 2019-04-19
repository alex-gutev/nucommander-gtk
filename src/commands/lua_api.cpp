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

#include "lua_api.h"

#include "luawrapper/luawrapper.hpp"

#include "tasks/async_task.h"

#include "interface/app_window.h"
#include "interface/file_view.h"
#include "commands/commands.h"

#include "directory/dir_entry.h"

using namespace nuc;

/* Application Functions */

/**
 * Execute Command Function.
 *
 * Arguments:
 *
 * - String: Command Name
 *
 * - NucWindow: The window in which the command should be executed.
 *
 * - NucPane: The source pane in which the command should be executed.
 *
 * Return Values: None
 */
static int exec_command(lua_State *L);

/**
 * Open a file with a given application.
 *
 * Arguments:
 *
 *  - String: Application
 *  - String: File Path
 *
 * Return Values: None
 */
static int open_with(lua_State *L);

/**
 * Launches an application
 *
 * Arguments:
 *
 *  1. String: Application
 *
 *  .. Strings: Command-line arguments.
 *
 * Return Values: None
 */
static int launch(lua_State *L);

/** Nuc Functions Table */
static luaL_Reg Nuc_table[] = {
    {"exec_command", exec_command},
    {"open_with", open_with},
    {"launch", launch},
    {NULL, NULL}
};

/**
 * Creates the "Nuc" table containing the application functions in
 * Nuc_table.
 *
 * @param L Lua interpreter state.
 */
static void create_nuc_table(lua_State *L);

/**
 * Launches an application.
 *
 * @param app Name of the application to launch.
 */
static void launch_app(const std::string &app);
/**
 * Launches an application with a file passed as an argument.
 *
 * @param file The file passed as an argument.
 */
static void launch_app_with_file(const std::string &app, const std::string &file);


/* NucWindow Methods */

/**
 * Unpacks a file to a temporary location (if necessary), and calls a
 * Lua callback function, with the full path to the file passed as an
 * argument, once it has been unpacked.
 *
 * The callback function should not depend that the values of the
 * window and source_pane variables are the same as when the command
 * script was executed, nor should it even depend that the window,
 * source pane, or entry objects are still in memory.
 *
 * Arguments:
 *
 * - NucWindow: The window, on the task queue of which, to queue the
 *      unpack task.
 *
 * - NucPane: The pane containing the entry.
 *
 * - NucEntry: The entry to unpack.
 *
 * - Function: Callback function which is called with one argument the
 *      full path to the unpacked file, once the file has been
 *      successfully unpacked.
 */
static int window_unpack_file(lua_State *L);

/**
 * NucWindow (app_window wrapper) table and metatable.
 */
static luaL_Reg NucWindow_table[] = {
    {NULL, NULL}
};
static luaL_Reg NucWindow_metatable[] = {
    {"unpack_file", window_unpack_file},
    {NULL, NULL}
};


/* NucPane Methods */

/**
 * Get the selected entry.
 *
 * Return Values:
 *
 * - The selected NucEntry or nil if there is no selected entry.
 */
static int pane_selected(lua_State *L);

/**
 * Get the logical path to the current directory of the pane.
 *
 * Return Values:
 *
 * - Path string.
 */
static int pane_path(lua_State *L);

/**
 * NucPane (file_view wrapper) table and metatable.
 */
static luaL_Reg NucPane_table[] = {
    {NULL, NULL}
};
static luaL_Reg NucPane_metatable[] = {
    {"selected", pane_selected},
    {"path", pane_path},
    {NULL, NULL}
};


/* NucEntry Methods */

/**
 * Get the file name.
 *
 * Return Values:
 *  - String
 */
static int entry_name(lua_State *L);
/**
 * Get the file extension (the substring of the file name following
 * the last '.').
 *
 * Return Values:
 *  - String
 */
static int entry_extension(lua_State *L);
/**
 * Get the entry type.
 *
 * Return Values:
 *
 * - Integer: Integer corresponding to a file type in NucEntry
 *     (NucEntry.TYPE_x)
 */
static int entry_type(lua_State *L);
/**
 * Get the file type of the underlying file.
 *
 * This differs from the entry type in that if the entry is a symbolic
 * link, this method returns the type of the target file (if it
 * exists) whereas entry_type returns NucEntry.TYPE_LNK (symbolic
 * link).
 *
 * Return Values:
 *
 * - Integer: Integer corresponding to a file type in NucEntry
 *     (NucEntry.TYPE_x)
 */
static int entry_file_type(lua_State *L);

/** NucEntry (dir_entry wrapper) table and metatable */
static luaL_Reg NucEntry_table[] = {
    {NULL, NULL}
};
static luaL_Reg NucEntry_metatable[] = {
    {"name", entry_name},
    {"extension", entry_extension},
    {"type", entry_type},
    {"file_type", entry_file_type},
    {NULL, NULL}
};

/**
 * Add the file type constants to the NucEntry table.
 *
 * The file type constants are prefixed with TYPE_.
 *
 * @param L Lua interpreter state.
 */
static void add_type_constants(lua_State *L);


//// API Registration Functions

void nuc::register_nuc_api(lua_State *L) {
    create_nuc_table(L);

    luaW_register<app_window>(L, "NucWindow", NucWindow_table, NucWindow_metatable, NULL, NULL);
    luaW_register<file_view>(L, "NucPane", NucPane_table, NucPane_metatable, NULL, NULL);
    luaW_register<dir_entry>(L, "NucEntry", NucEntry_table, NucEntry_metatable, NULL, NULL);

    add_type_constants(L);
}

void create_nuc_table(lua_State *L) {
    luaL_newlib(L, Nuc_table);
    lua_setglobal(L, "Nuc");
}

void add_type_constants(lua_State *L) {
    lua_getglobal(L, "NucEntry");

    lua_pushinteger(L, dir_entry::type_unknown);
    lua_setfield(L, -2, "TYPE_UNKNOWN");

    lua_pushinteger(L, dir_entry::type_fifo);
    lua_setfield(L, -2, "TYPE_FIFO");

    lua_pushinteger(L, dir_entry::type_chr);
    lua_setfield(L, -2, "TYPE_CHR");

    lua_pushinteger(L, dir_entry::type_reg);
    lua_setfield(L, -2, "TYPE_REG");

    lua_pushinteger(L, dir_entry::type_lnk);
    lua_setfield(L, -2, "TYPE_LNK");

    lua_pushinteger(L, dir_entry::type_sock);
    lua_setfield(L, -2, "TYPE_SOCK");

    lua_pushinteger(L, dir_entry::type_wht);
    lua_setfield(L, -2, "TYPE_WHT");

    lua_pushinteger(L, dir_entry::type_parent);
    lua_setfield(L, -2, "TYPE_PARENT");

    lua_pop(L, 1);
}

void nuc::pass_lua_command_args(lua_State *L, app_window *window, file_view *src) {
    lua_getglobal(L, "Nuc");

    luaW_push<app_window>(L, window);
    lua_setfield(L, -2, "window");

    luaW_push<file_view>(L, src);
    lua_setfield(L, -2, "source");
}


//// API Functions

/// Application Functions

int exec_command(lua_State *L) {
    const char *cmd = luaL_checkstring(L, 1);

    app_window *window = luaW_check<app_window>(L, 2);
    file_view *view = luaW_check<file_view>(L, 3);

    command_keymap::instance().exec_command(cmd, window, view, nullptr);

    return 0;
}

int open_with(lua_State *L) {
    const char *app = luaL_checkstring(L, 1);
    const char *file = luaL_checkstring(L, 2);

    launch_app_with_file(app, file);

    return 0;
}

int launch(lua_State *L) {
    const char *app = luaL_checkstring(L, 1);

    std::string cmd = app;

    int nargs = lua_gettop(L);
    for (int i = 2; i <= nargs; ++i) {
        cmd.append(" ");
        cmd.append(luaL_checkstring(L, i));
    }

    launch_app(cmd);

    return 0;
}


// Application Launch Utilities

static void launch_app(const std::string &app) {
#ifdef __APPLE__
    Glib::spawn_command_line_async(Glib::ustring::compose("open -a %1", app));

#else
    auto info = Gio::AppInfo::create_from_commandline(app, "", Gio::AppInfoCreateFlags::APP_INFO_CREATE_NONE);

    try {
        if (info)
            info->launch_uri("");
    }
    catch (const Glib::Error &) {
        // For now do nothing
    }

#endif
}

static void launch_app_with_file(const std::string &app, const std::string &file) {
#ifdef __APPLE__
    Glib::spawn_command_line_async(Glib::ustring::compose("open -a %1 '%2'", app, file));

#else
    auto info = Gio::AppInfo::create_from_commandline(app, "", Gio::AppInfoCreateFlags::APP_INFO_CREATE_NONE);

    try {
        if (info)
            info->launch(Gio::File::create_for_path(file));
    }
    catch (const Glib::Error &) {
        // For now do nothing
    }

#endif
}


/// NucWindow Methods

int window_unpack_file(lua_State *L) {
    app_window *window = luaW_check<app_window>(L, 1);
    file_view *src = luaW_check<file_view>(L, 2);
    dir_entry *ent = luaW_check<dir_entry>(L, 3);

    if (!lua_isfunction(L, 4))
        luaL_argerror(L, 4, "Argument 4 to NucWindow:unpack_file is expected to be a function.");

    // Store reference to function, which is at the top of the stack.
    int fn_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    window->add_operation(src->file_list()->dir_vfs()->access_file(*ent, [=] (const pathname &path) {
        dispatch_main([=] {
            // Get function
            lua_rawgeti(L, LUA_REGISTRYINDEX, fn_ref);
            lua_pushstring(L, path.c_str());
            lua_pcall(L, 1, 0, 0);

            luaL_unref(L, LUA_REGISTRYINDEX, fn_ref);
        });
    }));

    return 0;
}


/// NucPane Methods

int pane_selected(lua_State *L) {
    // Pane
    file_view *pane = luaW_check<file_view>(L, 1);

    if (dir_entry *ent = pane->selected_entry())
        luaW_push<dir_entry>(L, ent);
    else
        lua_pushnil(L);

    return 1;
}

int pane_path(lua_State *L) {
    file_view *pane = luaW_check<file_view>(L, 1);
    lua_pushstring(L, pane->path().c_str());

    return 1;
}


/// NucEntry Methods

int entry_name(lua_State *L) {
    dir_entry *ent = luaW_check<dir_entry>(L, 1);
    lua_pushstring(L, ent->file_name().c_str());

    return 1;
}
int entry_extension(lua_State *L) {
    dir_entry *ent = luaW_check<dir_entry>(L, 1);
    lua_pushstring(L, ent->subpath().extension().c_str());

    return 1;
}

int entry_type(lua_State *L) {
    dir_entry *ent = luaW_check<dir_entry>(L, 1);
    lua_pushinteger(L, ent->ent_type());

    return 1;
}
int entry_file_type(lua_State *L) {
    dir_entry *ent = luaW_check<dir_entry>(L, 1);
    lua_pushinteger(L, ent->type());

    return 1;
}
