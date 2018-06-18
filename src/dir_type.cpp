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

#include "dir_type.h"

#include "dir_lister.h"
#include "archive_lister.h"

#include "archive_tree.h"

#include "archive_plugin_loader.h"

static nuc::lister * make_dir_lister() {
    return new nuc::dir_lister();
}

static nuc::lister * make_archive_lister(nuc::archive_plugin *plugin) {
    return new nuc::archive_lister(plugin);
}

static nuc::dir_tree * make_dir_tree(nuc::path_str subpath) {
    return new nuc::dir_tree();
}

static nuc::dir_tree * make_archive_tree(nuc::path_str subpath) {
    return new nuc::archive_tree(subpath);
}


nuc::dir_type nuc::dir_type::get(path_str path) {
    if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(path)) {
        return dir_type(std::move(path), std::bind(make_archive_lister, plugin), make_archive_tree, false, "");
    }

    return dir_type(std::move(path), make_dir_lister, make_dir_tree, true, "");
}

nuc::dir_type nuc::dir_type::get(path_str path, const dir_entry& ent) {
    switch (ent.type()) {
    case DT_DIR:
        append_component(path, ent.file_name());
        return dir_type(path, make_dir_lister, make_dir_tree, true, "");

    case DT_REG:
        if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(ent.file_name())) {
            append_component(path, ent.file_name());
            return dir_type(path, std::bind(make_archive_lister, plugin), make_archive_tree, false, "");
        }
    }
    
    return dir_type();
}

// Local Variables:
// indent-tabs-mode: nil
// End:
