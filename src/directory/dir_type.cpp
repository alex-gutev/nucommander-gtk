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
#include "stream/sub_archive_dir_writer.h"

using namespace nuc;


//// Directory Types

/**
 * Regular directory type.
 *
 * For on disk directories readable directly using the OS's file
 * system API.
 */
class reg_dir_type : public dir_type {
    pathname m_path;

public:
    reg_dir_type(pathname path) : m_path(path) {}

    virtual lister * create_lister() const {
        return new dir_lister(m_path);
    }

    virtual tree_lister * create_tree_lister(const std::vector<pathname> &subpaths) const {
        return new dir_tree_lister(m_path, subpaths);
    }

    virtual dir_tree * create_tree() const {
        return new dir_tree();
    }

    virtual dir_writer * create_writer() const {
        return new reg_dir_writer(m_path.path().c_str());
    }

    virtual bool is_dir() const noexcept {
        return true;
    }

    virtual pathname path() const {
        return m_path;
    }

    virtual void subpath(const pathname &subpath) {}

    virtual pathname logical_path() const {
        return m_path;
    }

    virtual std::shared_ptr<dir_type> change_subpath(const pathname &pathname) const {
        // This method should never be called
        assert(false);

        return nullptr;
    }
};

/**
 * Archive directory type.
 */
class archive_dir_type : public dir_type {
    archive_plugin * plugin;

    pathname m_path;
    pathname m_subpath;

public:
    archive_dir_type(archive_plugin *plugin, pathname path, pathname subpath)
        : plugin(plugin), m_path(path), m_subpath(subpath) {}

    virtual lister * create_lister() const {
        plugin->load();
        return new archive_lister(plugin, m_path);
    }

    virtual tree_lister * create_tree_lister(const std::vector<pathname> &subpaths) const {
        plugin->load();
        return new archive_tree_lister(plugin, m_path, subpaths);
    }

    virtual dir_tree * create_tree() const {
        return new archive_tree(m_subpath);
    }

    virtual dir_writer * create_writer() const {
        return new archive_dir_writer(m_path.path().c_str(), plugin, m_subpath);
    }

    virtual bool is_dir() const noexcept {
        return false;
    }

    virtual pathname path() const {
        return m_path;
    }

    virtual pathname subpath() const {
        return m_subpath;
    }

    virtual pathname logical_path() const {
        return m_path.append(m_subpath);
    }

    virtual std::shared_ptr<dir_type> change_subpath(const pathname &subpath) const {
        auto type = std::make_shared<archive_dir_type>(*this);
        type->m_subpath = subpath;

        return type;
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
    pathname m_path;
    /**
     * Subpath within the archive.
     */
    pathname m_subpath;

public:
    sub_archive_dir_type(archive_plugin *plugin, std::shared_ptr<dir_type> parent_type, const pathname &path, const pathname &subpath)
        : plugin(plugin), parent_type(parent_type->change_subpath("")), m_path(path), m_subpath(subpath) {
    }


    virtual archive_lister * create_lister() const {
        plugin->load();

        return new sub_archive_lister(parent_type->create_lister(), plugin, m_path);
    }

    virtual tree_lister * create_tree_lister(const std::vector<pathname> &subpaths) const {
        return new archive_tree_lister(create_lister(), subpaths);
    }

    virtual dir_tree * create_tree() const {
        return new archive_tree(m_subpath);
    }

    virtual dir_writer *create_writer() const {
        return new sub_archive_dir_writer(plugin, new sub_archive_dir_type(*this), parent_type->create_writer(), m_path, m_subpath);
    }

    virtual bool is_dir() const noexcept {
        return false;
    }

    virtual pathname path() const {
        return parent_type->path();
    }

    virtual pathname subpath() const {
        return m_subpath;
    }

    virtual pathname logical_path() const {
        return parent_type->logical_path().append(m_path).append(m_subpath);
    }

    virtual std::shared_ptr<dir_type> change_subpath(const pathname &subpath) const {
        auto type = std::make_shared<sub_archive_dir_type>(*this);
        type->m_subpath = subpath;

        return type;
    }
};


//// Path Canonicalization Utilities

/**
 * Canonicalizes a path by expanding leading tilde's and
 * removing all '.' and '..' directory components.
 *
 * @param path The path to canonicalize.
 *
 * @return The canonicalized path.
 */
static pathname canonicalize(const pathname &path) {
    return path.expand_tilde().canonicalize();
}

/**
 * Searches the directory, at @a dir, for an entry with a name
 * that is equal to @a comp or the first entry with a name
 * that matches, ignoring case, @a comp.
 *
 * @return The name of the matching entry or an empty string
 *    if the directory @a dir could not be read.
 */
static pathname::string find_match_comp(const pathname::string &dir, const pathname::string &comp) {
    if (comp == "/")
        return comp;

    try {
        dir_lister lister(dir);
        lister::entry ent;

        pathname::string match;

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

/**
 * Canonicalizes the case of a path.
 *
 * Each intermediate directory component is searched for an
 * entry with a name which is either identical to the child
 * component, in which case the the case is preserved, or the
 * first entry with a name which matches (ignoring case) the
 * name of the child component, in which case the child is
 * replaced with the matching entry.
 *
 * @return A pair where the first value is the path with the
 *    case canonicalized up to the last directory component
 *    which can be read, the second value is the remainder of
 *    the path, which has not been case-canonicalized.
 */
static std::pair<pathname, pathname> canonicalize_case(const pathname &path) {
    pathname cpath;
    auto comps(path.components());

    for (auto it = comps.begin(), end = comps.end(); it != end; ++it) {
        const pathname::string &comp = *it;

        pathname::string can_comp = find_match_comp(cpath, comp);

        // Directory could not be read
        if (can_comp.empty()) {
            return std::make_pair(cpath, pathname(std::vector<pathname::string>(it, end), path.is_dir()));
        }

        cpath = cpath.append(can_comp);
    }

    return std::make_pair(cpath, "");
}


/**
 * Determines which initial components of a path refer to an
 * existing file.
 *
 * @param path The path to check.
 *
 * @return A pair where the first value is the initial portion
 *   of path which refers to an existing file and the second
 *   value is the remaining non-existent portion.
 */
static std::pair<pathname::string, pathname::string> find_dir(const pathname::string &path) {
    pathname::string sub_path = path;
    size_t sep_index = pathname::string::npos;

    while (!sub_path.empty() && access(sub_path.c_str(), F_OK)) {
        sep_index = sub_path.rfind('/', sep_index);

        if (sep_index != pathname::string::npos) {
            sub_path = path.substr(0, sep_index);
        }
    }

    return std::make_pair(
        !sub_path.empty() ? sub_path : "/",
        sep_index != pathname::string::npos ? path.substr(sep_index + 1, path.length() - sep_index) : pathname::string()
    );
}

/**
 * Checks whether the file @a path is a regular directory.
 *
 * @path Path to the file.
 *
 * @return True if the file is a regular directory.
 */
static bool is_reg_dir(const pathname::string &path) {
    struct stat st;

    return !stat(path.c_str(), &st) && S_ISDIR(st.st_mode);
}


//// Public Static Methods

/// Getting a dir_type object

/**
 * Determines the directory type for the sub-directory @a dir nested
 * within the archive with type @a dtype.
 *
 * If @a dir is a directory within the archive, @a dtype is returned
 * with its subpath set to @a dir.
 *
 * If @a dir is an archive file stored within the archive, a new
 * nested archive directory type is created (for the nested archive)
 * and returned.
 *
 * @param dtype Directory type of the containing archive.
 * @param dir Directory within the archive.'
 *
 * @return dir_type object of the archive directory.
 */
static std::shared_ptr<dir_type> get_archive_type(std::shared_ptr<dir_type> dtype, const pathname &dir);

/**
 * Searches the archive with type @a dtype for a regular file entry
 * whose name is either equal to or a parent component of @a dir.
 *
 * @param dtype Type of the containing archive.
 * @param dir Archive subpath.
 *
 * @return A pair where the first entry is the path to the archive
 *   file found and the second entry is the remaining components of
 *   the subpath, i.e. the subpath within the archive file.
 */
static std::pair<pathname, pathname> find_archive_file(std::shared_ptr<dir_type> dtype, const pathname &dir);


std::shared_ptr<dir_type> dir_type::get(const pathname &path) {
    auto pair = canonicalize_case(canonicalize(path));

    if (!pair.second.empty() || !is_reg_dir(pair.first)) {
        if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(pair.first)) {
            return get_archive_type(std::make_shared<archive_dir_type>(plugin, pair.first, ""), pair.second);
        }
    }

    return std::make_shared<reg_dir_type>(pair.first.append(pair.second));
}

std::shared_ptr<dir_type> get_archive_type(std::shared_ptr<dir_type> dtype, const pathname &dir) {
    pathname file, subpath;

    if (!dir.empty()) {
        std::tie(file, subpath) = find_archive_file(dtype, dir);

        if (!file.empty()) {
            if (archive_plugin *plugin = archive_plugin_loader::instance().get_plugin(file.basename())) {
                return get_archive_type(std::make_shared<sub_archive_dir_type>(plugin, dtype, file, ""), subpath);
            }
        }

        return dtype->change_subpath(dir);
    }

    return dtype;
}

std::pair<pathname, pathname> find_archive_file(std::shared_ptr<dir_type> dtype, const pathname &dir) {
    // Longest subpath of dir that is actually in the archive.
    pathname subpath;

    std::unique_ptr<lister> listr(dtype->create_lister());
    lister::entry ent;

    while (listr->read_entry(ent)) {
        pathname name = ent.name;
        name = name.canonicalize();

        if (dir == name) {
            return std::make_pair(ent.type == DT_REG ? dir : "", "");
        }

        // If the current is a subpath of dir, then dir is a directory
        // in the archive.
        if (name.is_subpath(dir)) {
            return std::make_pair("", "");
        }

        if (dir.is_subpath(name) && ent.type == DT_REG) {
            // If name is longer than the previous subpath
            if (name.path().length() > subpath.path().length()) {
                subpath = name;
            }
        }
    }

    return subpath.empty() ? std::make_pair("","") : std::make_pair(subpath, dir.path().substr(subpath.path().length() + 1));
}


std::shared_ptr<dir_type> dir_type::get(const pathname &path, const dir_entry& ent) {
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


/// Getting a directory writer object

dir_writer * dir_type::get_writer(const pathname &path) {
    return get(path)->create_writer();
}


/// Querying Directory Type Properties

dir_type::fs_type dir_type::on_same_fs(const pathname::string &dir1, const pathname::string &dir2) {
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

pathname dir_type::get_subpath(const pathname &path) {
    return find_dir(path).second;
}


// Local Variables:
// indent-tabs-mode: nil
// End:
