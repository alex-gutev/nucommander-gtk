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

#include <boost/algorithm/string.hpp>

#include "dir_lister.h"
#include "archive_lister.h"

#include "archive_tree.h"
#include "archive_plugin_loader.h"

#include "path_utils.h"


/**
 * Creates a dir_lister object.
 *
 * @return The dir_lister object.
 */
static nuc::lister * make_dir_lister() {
    return new nuc::dir_lister();
}

/**
 * Creates an archive_lister object initialized with the archive
 * plugin @a plugin.
 *
 * @param plugin The archive plugin.
 *
 * @return The archive_lister object.
 */
static nuc::lister * make_archive_lister(nuc::archive_plugin *plugin) {
    return new nuc::archive_lister(plugin);
}

/**
 * Loads an archive plugin and creates an archhive_lister object
 * initialized with that plugin.
 *
 * @param plugin The plugin to load (as returned by
 *    archive_plugin_loader::get_plugin).
 *
 * @return The archive_lister object.
 */
static nuc::lister * load_archive_lister(nuc::archive_plugin_loader::plugin *plugin) {
    return make_archive_lister(plugin->load());
}

/**
 * Creates a dir_tree object.
 *
 * @param subpath Ignored as dir_tree objects don't have
 *    subdirectories.
 *
 * @return The dir_tree object.
 */
static nuc::dir_tree * make_dir_tree(nuc::path_str subpath) {
    return new nuc::dir_tree();
}

/**
 * Creates an archive_tree object with the initial subdirectory at @a
 * subpath.
 *
 * @param subpath The subpath of the initial subdirectory.
 *
 * @return The archive_tree object.
 */
static nuc::dir_tree * make_archive_tree(nuc::path_str subpath) {
    return new nuc::archive_tree(subpath);
}


/// Private methods

nuc::path_str nuc::dir_type::canonicalize(const path_str &path) {
    return canonicalized_path(expand_tilde(path));
}

nuc::path_str nuc::dir_type::find_match_comp(const path_str &dir, const path_str &comp) {
    if (comp == "/")
        return comp;

    try {
        dir_lister lister;
        lister::entry ent;

        lister.open(dir);

        path_str match;

        while (lister.read_entry(ent)) {
            if (comp == ent.name) {
                return comp;
            }
            else if (match.empty() && boost::iequals(comp, ent.name)) {
                match = ent.name;
            }
        }

        return match.empty() ? comp : match;

    } catch(lister::error &e) {
        return "";
    }
}

std::pair<nuc::path_str, nuc::path_str> nuc::dir_type::canonicalize_case(const path_str &path) {
    path_str cpath;
    path_components comps(path);

    for (auto it = comps.begin(), end = comps.end(); it != end; ++it) {
        const path_str &comp = *it;

        path_str can_comp = find_match_comp(cpath, comp);

        // Directory could not be read
        if (can_comp.empty()) {
            return std::make_pair(cpath, path.substr(it.position()));
        }

        append_component(cpath, can_comp);
    }

    return std::make_pair(cpath, "");
}


/// Public static methods

nuc::dir_type nuc::dir_type::get(const path_str &path) {
    path_str cpath = canonicalize(path);
    std::pair<path_str, path_str> pair = canonicalize_case(cpath);

    if (archive_plugin_loader::plugin *plugin = archive_plugin_loader::instance().get_plugin(pair.first)) {
        return dir_type(std::move(pair.first), std::bind(make_archive_lister, plugin->load()), make_archive_tree, false, std::move(pair.second));
    }

    if (!pair.second.empty())
        append_component(pair.first, pair.second);

    return dir_type(std::move(pair.first), make_dir_lister, make_dir_tree, true, "");
}

nuc::dir_type nuc::dir_type::get(path_str path, const dir_entry& ent) {
    switch (ent.type()) {
    case DT_DIR:
        append_component(path, ent.file_name());
        return dir_type(path, make_dir_lister, make_dir_tree, true, "");

    case DT_REG:
        if (archive_plugin_loader::plugin *plugin = archive_plugin_loader::instance().get_plugin(ent.file_name())) {
            append_component(path, ent.file_name());
            return dir_type(path, std::bind(load_archive_lister, plugin), make_archive_tree, false, "");
        }
    }
    
    return dir_type();
}

// Local Variables:
// indent-tabs-mode: nil
// End:
