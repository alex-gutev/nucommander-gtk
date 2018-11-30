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

#include "move.h"

#include <memory>

#include "errors/restarts.h"

#include "stream/dir_writer.h"

using namespace nuc;

task_queue::task_type nuc::make_move_task(dir_type src_type, const std::vector<dir_entry*> &entries, const paths::string &dest) {
    std::vector<paths::string> paths;

    for (dir_entry *ent : entries) {
        paths.push_back(ent->subpath());

        if (ent->type() == dir_entry::type_dir)
            paths.back() += '/';
    }

    return [=] (cancel_state &state) {
        std::unique_ptr<dir_writer> writer(dir_type::get_writer(src_type.path()));

        try {
            move(state, paths, dest, *writer);
        }
        catch (const error &e) {
            // Catch error to abort operation.
        }
    };
}

void nuc::move(cancel_state &state, const std::vector<paths::string> &items, const paths::string &dest, dir_writer &dir) {
    for (const paths::string &item : items) {
        global_restart skip(skip_exception::restart);

        try {
            dir.rename(item.c_str(), paths::appended_component(dest, item).c_str());
        }
        catch (const skip_exception &) {
            // Do nothing to skip the current file
        }
    }
}
