/*
 * NuCommander
 * Copyright (C) 2018-2019  Alexander Gutev <alex.gutev@gmail.com>
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
#include "lister/sub_archive_lister.h"

#include "archive_tree.h"
#include "plugins/archive_plugin_loader.h"

#include "stream/reg_dir_writer.h"
#include "stream/archive_dir_writer.h"

using namespace nuc;

/**
 * Regular directory type.
 *
 * For on disk directories readable directly using the OS's file
 * system API.
 */
class reg_dir_type : public dir_type {
    paths::pathname m_path;

public:
    reg_dir_type(paths::pathname path) : m_path(path) {}

    virtual std::shared_ptr<dir_type> copy() const {
        return std::make_shared<reg_dir_type>(*this);
    }

    virtual lister * create_lister() const {
        return new dir_lister(m_path);
    }

    virtual tree_lister * create_tree_lister(const std::vector<paths::pathname> &subpaths) const {
        return new dir_tree_lister(m_path, subpaths);
    }

    virtual dir_tree * create_tree() const {
        return new dir_tree();
    }

    virtual bool is_dir() const {
        return true;
    }

    virtual paths::pathname path() const {
        return m_path;
    }

    virtual paths::pathname subpath() const {
        return "";
    }

    virtual void subpath(const paths::pathname &subpath) {}

    virtual paths::pathname logical_path() const {
        return m_path;
    }
};

/**
 * Archive directory type.
 */
class archive_dir_type : public dir_type {
    archive_plugin * plugin;

    paths::pathname m_path;
    paths::pathname m_subpath;

public:
    archive_dir_type(archive_plugin *plugin, paths::pathname path, paths::pathname subpath)
        : plugin(plugin), m_path(path), m_subpath(subpath) {}

    virtual std::shared_ptr<dir_type> copy() const {
        return std::make_shared<archive_dir_type>(*this);
    }

    virtual lister * create_lister() const {
        plugin->load();
        return new archive_lister(plugin, m_path);
    }

    virtual tree_lister * create_tree_lister(const std::vector<paths::pathname> &subpaths) const {
        plugin->load();
        return new archive_tree_lister(plugin, m_path, subpaths);
    }

    virtual dir_tree * create_tree() const {
        return new archive_tree(m_subpath);
    }

    virtual bool is_dir() const {
        return false;
    }

    virtual paths::pathname path() const {
        return m_path;
    }

    virtual paths::pathname subpath() const {
        return m_subpath;
    }

    virtual void subpath(const paths::pathname &subpath) {
        m_subpath = subpath;
    }

    virtual paths::pathname logical_path() const {
        return m_path.append(m_subpath);
    }
};

/**
 * Archive nested in another archive directory type.
 */
class sub_archive_dir_type : public dir_type {
    /**
     * Plugin for reading the nested archive.
     */
    archive_plugin *plugin;

    /**
     * Directory type of the archive containing this archive.
     */
    std::shared_ptr<dir_type> parent_type;

    /**
     * Subpath to the archive within the containing archive.
     */
    paths::pathname m_path;
    /**
     * Subpath within the archive.
     */
    paths::pathname m_subpath;

public:
    sub_archive_dir_type(archive_plugin *plugin, std::shared_ptr<dir_type> parent_type, const paths::pathname &path, const paths::pathname &subpath)
        : plugin(plugin), parent_type(parent_type), m_path(path), m_subpath(subpath) {}

    virtual std::shared_ptr<dir_type> copy() const {
        return std::make_shared<sub_archive_dir_type>(*this);
    }


    virtual archive_lister * create_lister() const {
        plugin->load();

        return new sub_archive_lister(parent_type->create_lister(), plugin, m_path);
    }

    virtual tree_lister * create_tree_lister(const std::vector<paths::pathname> &subpaths) const {
        return new archive_tree_lister(create_lister(), subpaths);
    }

    virtual dir_tree * create_tree() const {
        return new archive_tree(m_subpath);
    }

    virtual bool is_dir() const {
        return false;
    }

    virtual paths::pathname path() const {
        return parent_type->path();
    }

    virtual paths::pathname subpath() const {
        return m_subpath;
    }

    virtual void subpath(const paths::pathname &subpath) {
        m_subpath = subpath;
    }

    virtual paths::pathname logical_path() const {
        return parent_type->logical_path().append(m_path).append(m_subpath);
    }
};


/// Path Canonicalization Utilities

static paths::pathname canonicalize(const paths::pathname &path) {
    return path.expand_tilde().canonicalize();
}

static paths::string find_match_comp(const paths::string &dir, const paths::string &comp) {
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

    } catch(const error &e) {
        return "";
    }
}

static std::pair<paths::pathname, paths::pathname> canonicalize_case(const paths::pathname &path) {
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


static std::pair<paths::string, paths::string> find_dir(const paths::string &path) {
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

static bool is_reg_dir(const paths::string &path) {
    struct stat st;

    return !stat(path.c_str(), &st) && S_ISDIR(st.st_mode);
}


/// Public static methods

// Getting a dir_type object

std::shared_ptr<dir_type> dir_type::get(const paths::pathname &path) {
    auto pair = canonicalize_case(canonicalize(path));

    if (!pair.second.empty() || !is_reg_dir(pair.first)) {
        if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(pair.first)) {
            return std::make_shared<archive_dir_type>(plugin, pair.first, pair.second);
        }
    }

    return std::make_shared<reg_dir_type>(pair.first.append(pair.second));
}

std::shared_ptr<dir_type> dir_type::get(const paths::pathname &path, const dir_entry& ent) {
    switch (ent.type()) {
    case dir_entry::type_dir:
        return std::make_shared<reg_dir_type>(path.append(ent.file_name()));

    case dir_entry::type_reg:
        if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(ent.file_name())) {
            return std::make_shared<archive_dir_type>(plugin, path.append(ent.file_name()), "");
        }

    default:
        return nullptr;

    }
}

std::shared_ptr<dir_type> dir_type::get(std::shared_ptr<dir_type> dir, const nuc::dir_entry &ent) {
    if (dir->is_dir()) {
        return get(dir->path(), ent);
    }
    else {
        if (ent.type() == dir_entry::type_reg) {
            if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(ent.file_name())) {
                return std::make_shared<sub_archive_dir_type>(plugin, dir, ent.subpath(), "");
            }
        }
    }

    return nullptr;
}

// Getting a directory writer object

dir_writer * dir_type::get_writer(const paths::pathname &path) {
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

dir_type::fs_type dir_type::on_same_fs(const paths::string &dir1, const paths::string &dir2) {
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

paths::pathname dir_type::get_subpath(const paths::pathname &path) {
    return find_dir(path).second;
}


// Local Variables:
// indent-tabs-mode: nil
// End:
