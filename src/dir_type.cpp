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

static std::pair<nuc::lister*, nuc::dir_tree*> make_dir_lister() {
    return std::make_pair(new nuc::dir_lister(), new nuc::dir_tree());
}

static std::pair<nuc::lister*, nuc::dir_tree*> make_archive_lister(nuc::archive_plugin *plugin) {
    return std::make_pair(new nuc::archive_lister(plugin), new nuc::archive_tree());
}


std::pair<nuc::lister*, nuc::dir_tree*> nuc::get_lister(const path_str& path) {
    if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(path)) {
        return make_archive_lister(plugin);
    }

    return make_dir_lister();
}

nuc::create_lister_fn nuc::get_lister_fn(const dir_entry& ent) {
    switch (ent.type()) {
    case DT_DIR:
        return make_dir_lister;

    case DT_REG:
        if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(ent.file_name())) {
            return std::bind(make_archive_lister, plugin);
        }
    }
    
    return nullptr;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
