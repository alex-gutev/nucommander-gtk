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

#include "lister/dir_lister.h"
#include "lister/archive_lister.h"

#include "archive_tree.h"
#include "plugins/archive_plugin_loader.h"

#include "paths/utils.h"


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
    plugin->load();
    return new nuc::archive_lister(plugin);
}

/**
 * Creates a dir_tree object.
 *
 * @param subpath Ignored as dir_tree objects don't have
 *    subdirectories.
 *
 * @return The dir_tree object.
 */
static nuc::dir_tree * make_dir_tree(nuc::paths::string subpath) {
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
static nuc::dir_tree * make_archive_tree(nuc::paths::string subpath) {
    return new nuc::archive_tree(subpath);
}


/// Private methods

nuc::paths::string nuc::dir_type::canonicalize(const paths::string &path) {
    return paths::canonicalized_path(paths::expand_tilde(path));
}

nuc::paths::string nuc::dir_type::find_match_comp(const paths::string &dir, const paths::string &comp) {
    if (comp == "/")
        return comp;

    try {
        dir_lister lister;
        lister::entry ent;

        lister.open(dir);

        paths::string match;

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

std::pair<nuc::paths::string, nuc::paths::string> nuc::dir_type::canonicalize_case(const paths::string &path) {
    paths::string cpath;
    paths::path_components comps(path);

    for (auto it = comps.begin(), end = comps.end(); it != end; ++it) {
        const paths::string &comp = *it;

        paths::string can_comp = find_match_comp(cpath, comp);

        // Directory could not be read
        if (can_comp.empty()) {
            return std::make_pair(cpath, path.substr(it.position()));
        }

        paths::append_component(cpath, can_comp);
    }

    return std::make_pair(cpath, "");
}


/// Public static methods

nuc::dir_type nuc::dir_type::get(const paths::string &path) {
    paths::string cpath = canonicalize(path);
    std::pair<paths::string, paths::string> pair = canonicalize_case(cpath);

    if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(pair.first)) {
        return dir_type(std::move(pair.first), std::bind(make_archive_lister, plugin), make_archive_tree, false, std::move(pair.second));
    }

    if (!pair.second.empty())
        paths::append_component(pair.first, pair.second);

    return dir_type(std::move(pair.first), make_dir_lister, make_dir_tree, true, "");
}

nuc::dir_type nuc::dir_type::get(paths::string path, const dir_entry& ent) {
    switch (ent.type()) {
    case dir_entry::type_dir:
        paths::append_component(path, ent.file_name());
        return dir_type(path, make_dir_lister, make_dir_tree, true, "");

    case dir_entry::type_reg:
        if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(ent.file_name())) {
            paths::append_component(path, ent.file_name());
            return dir_type(path, std::bind(make_archive_lister, plugin), make_archive_tree, false, "");
        }

    default:
        return dir_type();

    }
}

// Local Variables:
// indent-tabs-mode: nil
// End:
