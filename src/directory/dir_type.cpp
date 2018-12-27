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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <boost/algorithm/string.hpp>

#include "lister/dir_lister.h"
#include "lister/archive_lister.h"

#include "lister/dir_tree_lister.h"
#include "lister/archive_tree_lister.h"

#include "archive_tree.h"
#include "plugins/archive_plugin_loader.h"

#include "stream/reg_dir_writer.h"
#include "stream/archive_dir_writer.h"

#include "paths/utils.h"


/**
 * Creates a dir_lister object.
 *
 * @param path Path to the directory
 *
 * @return The dir_lister object.
 */
static nuc::lister * make_dir_lister(const nuc::paths::pathname &path) {
    return new nuc::dir_lister(path);
}

/**
 * Creates a dir_tree_lister object.
 *
 * @param path Path to the directory.
 * @param subpaths Subpaths, within the directory, to list.
 *
 * @return The tree_lister object.
 */
static nuc::tree_lister * make_dir_tree_lister(const nuc::paths::pathname &dir, const std::vector<nuc::paths::pathname> &subpaths) {
    return new nuc::dir_tree_lister(dir, subpaths);
}

/**
 * Creates an archive_lister object initialized with the archive
 * plugin @a plugin.
 *
 * @param plugin The archive plugin.
 * @param path Path to the archive.
 *
 * @return The archive_lister object.
 */
static nuc::lister * make_archive_lister(nuc::archive_plugin *plugin, const nuc::paths::pathname &path) {
    plugin->load();
    return new nuc::archive_lister(plugin, path);
}

/**
 * Creates an archive_tree_lister object.
 *
 * @param path Path to the archive.
 * @param base Subpath within the archive.
 * @param subpaths Subpaths, within the @a base directory , to list.
 *
 * @return The tree_lister object.
 */
static nuc::tree_lister * make_archive_tree_lister(nuc::archive_plugin *plugin, const nuc::paths::pathname &base, const std::vector<nuc::paths::pathname> &subpaths) {
    return new nuc::archive_tree_lister(plugin, base, subpaths);
}

/**
 * Creates a dir_tree object.
 *
 * @param subpath Ignored as dir_tree objects don't have
 *    subdirectories.
 *
 * @return The dir_tree object.
 */
static nuc::dir_tree * make_dir_tree(const nuc::paths::pathname &subpath) {
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
static nuc::dir_tree * make_archive_tree(nuc::paths::pathname subpath) {
    return new nuc::archive_tree(std::move(subpath));
}


/// Private methods

nuc::paths::pathname nuc::dir_type::canonicalize(const paths::pathname &path) {
    return path.expand_tilde().canonicalize();
}

nuc::paths::string nuc::dir_type::find_match_comp(const paths::string &dir, const paths::string &comp) {
    if (comp == "/")
        return comp;

    try {
        dir_lister lister(dir);
        lister::entry ent;

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

std::pair<nuc::paths::pathname, nuc::paths::pathname> nuc::dir_type::canonicalize_case(const paths::pathname &path) {
    paths::pathname cpath;
    auto comps(path.components());

    for (auto it = comps.begin(), end = comps.end(); it != end; ++it) {
        const paths::string &comp = *it;

        paths::string can_comp = find_match_comp(cpath, comp);

        // Directory could not be read
        if (can_comp.empty()) {
            return std::make_pair(cpath, paths::pathname::from_components(std::vector<paths::string>(it, end), path.is_dir()));
        }

        cpath = cpath.append(can_comp);
    }

    return std::make_pair(cpath, "");
}


std::pair<nuc::paths::string, nuc::paths::string> nuc::dir_type::find_dir(const paths::string &path) {
    paths::string sub_path = path;
    size_t sep_index = paths::string::npos;

    while (!sub_path.empty() && access(sub_path.c_str(), F_OK)) {
        sep_index = sub_path.rfind('/', sep_index);

        if (sep_index != paths::string::npos) {
            sub_path = path.substr(0, sep_index);
        }
    }

    return std::make_pair(
        !sub_path.empty() ? sub_path : "/",
        sep_index != paths::string::npos ? path.substr(sep_index + 1, path.length() - sep_index) : paths::string()
    );
}


bool nuc::dir_type::is_reg_dir(const paths::string &path) {
    struct stat st;

    return !stat(path.c_str(), &st) && S_ISDIR(st.st_mode);
}

/// Public static methods

// Getting a dir_type object

nuc::dir_type nuc::dir_type::get(const paths::pathname &path) {
    using namespace std::placeholders;

    auto pair = canonicalize_case(canonicalize(path));

    if (!pair.second.empty() || !is_reg_dir(pair.first)) {
        if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(pair.first)) {
            return dir_type(std::move(pair.first),
                            std::bind(make_archive_lister, plugin, _1),
                            std::bind(make_archive_tree_lister, plugin, _1, _2),
                            make_archive_tree,
                            false,
                            std::move(pair.second));
        }
    }

    if (!pair.second.empty())
        pair.first = pair.first.append(pair.second);

    return dir_type(std::move(pair.first),
                    make_dir_lister,
                    make_dir_tree_lister,
                    make_dir_tree,
                    true,
                    "");
}

nuc::dir_type nuc::dir_type::get(const paths::pathname &path, const dir_entry& ent) {
    using namespace std::placeholders;

    switch (ent.type()) {
    case dir_entry::type_dir:
        return dir_type(path.append(ent.file_name()),
                        make_dir_lister,
                        make_dir_tree_lister,
                        make_dir_tree, true, "");

    case dir_entry::type_reg:
        if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(ent.file_name())) {
            return dir_type(path.append(ent.file_name()),
                            std::bind(make_archive_lister, plugin, _1),
                            std::bind(make_archive_tree_lister, plugin, _1, _2),
                            make_archive_tree,
                            false, "");
        }

    default:
        return dir_type();

    }
}


// Getting a directory writer object

nuc::dir_writer * nuc::dir_type::get_writer(const paths::pathname &path) {
    paths::pathname cpath = path.expand_tilde();
    std::pair<paths::string, paths::string> parts = find_dir(cpath);

    struct stat st;

    if (!stat(parts.first.c_str(), &st) && S_ISREG(st.st_mode)) {
        if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(parts.first)) {
            return new archive_dir_writer(parts.first.c_str(), plugin, parts.second.c_str());
        }
    }

    return new reg_dir_writer(cpath.path().c_str());
}


// Querying Directory Type Properties

nuc::dir_type::fs_type nuc::dir_type::on_same_fs(const paths::string &dir1, const paths::string &dir2) {
    auto path1 = find_dir(dir1).first;
    auto path2 = find_dir(dir2).first;

    struct stat st1, st2;

    if (!stat(path1.c_str(), &st1) && !stat(path2.c_str(), &st2)) {
        if (S_ISDIR(st1.st_mode) && S_ISDIR(st2.st_mode)) {
            return fs_type_dir;
        }
        else if (st1.st_ino == st2.st_ino && st1.st_dev == st2.st_dev) {
            return fs_type_virtual;
        }
    }

    return fs_type_none;
}


nuc::paths::pathname nuc::dir_type::get_subpath(const paths::pathname &path) {
    return find_dir(path).second;
}

// Local Variables:
// indent-tabs-mode: nil
// End:
