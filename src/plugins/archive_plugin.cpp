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

#include "archive_plugin.h"

#include <dlfcn.h>


#define LOAD_CHECK_FN(name) name = (name ## _fn)dlsym(dl_handle, "nuc_arch_"#name); check_error(error::api_incomplete);


using namespace nuc;

void archive_plugin::check_error(int code) {
    if (dlerror()) throw error(code);
}

void archive_plugin::load() {
    std::lock_guard<std::mutex> lock(mutex);

    if (!dl_handle) {
        if (!(dl_handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL))) {
            throw error(error::dlopen);
        }

        dlerror();

        LOAD_CHECK_FN(open);
        LOAD_CHECK_FN(close);
        LOAD_CHECK_FN(next_entry);
        LOAD_CHECK_FN(unpack);
        LOAD_CHECK_FN(copy_archive_type);
        LOAD_CHECK_FN(copy_last_entry);
        LOAD_CHECK_FN(create_entry);
        LOAD_CHECK_FN(pack);
        LOAD_CHECK_FN(set_callback);
    }
}

archive_plugin::~archive_plugin() {
    std::lock_guard<std::mutex> lock(mutex);

    if (dl_handle) dlclose(dl_handle);
}

// Local Variables:
// indent-tabs-mode: nil
// End:
