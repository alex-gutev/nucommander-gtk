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

#ifndef NUC_DIRECTORY_DIR_TYPE_H
#define NUC_DIRECTORY_DIR_TYPE_H

#include <tuple>
#include <memory>
#include <vector>

#include "types.h"
#include "paths/pathname.h"

#include "lister/lister.h"
#include "lister/tree_lister.h"

#include "dir_tree.h"
#include "dir_entry.h"

#include "stream/dir_writer.h"

/**
 * Functions for determining the lister and directory tree (dir_tree)
 * objects for a directory.
 */

namespace nuc {
    /**
     * Generic interface for creating directory listers.
     */
    class dir_type {
    public:
        /* Destructor */
        virtual ~dir_type() noexcept = default;

        /**
         * Creates a lister for the directory.
         *
         * @return lister object
         */
        virtual lister * create_lister() const = 0;

        /**
         * Creates a tree_lister for the directory.
         *
         * @param subpaths The subpaths to visit.
         *
         * @return tree_lister object
         */
        virtual tree_lister * create_tree_lister(const std::vector<pathname> &subpaths) const = 0;

        /**
         * Creates a directory tree suitable for the directory.
         *
         * @return dir_tree object.
         */
        virtual dir_tree * create_tree() const = 0;

        /**
         * Create a directory writer for the directory.
         *
         * @return dir_writer object
         */
        virtual dir_writer * create_writer() const = 0;

        /**
         * Returns true if the directory is a regular directory that
         * can be read directly using the OS's file system API.
         *
         * @return boolean
         */
        virtual bool is_dir() const noexcept = 0;

        /**
         * Returns the path to the directory file. If the directory is
         * located within a virtual file system, such as an archive,
         * the path to that file is returned instead.
         *
         * @return pathname
         */
        virtual pathname path() const = 0;

        /**
         * Returns the logical path to the directory that is the path
         * returned by subpath() concatenated to the path returned by
         * path().
         */
        virtual pathname logical_path() const = 0;

        /**
         * Returns a new dir_type object which is a copy of the
         * current dir_type object, however with the subpath changed
         * to @a subpath.
         *
         * @param subpath The subpath of the new dir_type object.
         *
         * @return The new dir_type object.
         */
        virtual std::shared_ptr<dir_type> change_subpath(const pathname &subpath) const = 0;

        /**
         * Determines the directory type of the directory @a path.
         *
         * The path @a path is canonicalized before being stored in
         * the dir_type object created, which can later be retrieved
         * using logical_path().
         *
         * @param path Path to the directory.
         */
        static std::shared_ptr<dir_type> get(const pathname &path);

        /**
         * Determines the directory type of the entry @a ent within
         * the directory at path @a path.
         *
         * Does not perform any blocking operations.
         *
         * @param path Path to the directory containing the entry.
         * @param ent  The entry within the directory @a path.
         */
        static std::shared_ptr<dir_type> get(const pathname &path, const dir_entry &ent);

        /**
         * Determines the directory type of the entry @a ent contained
         * within the directory with type @a dir.
         *
         * @param dir The directory type of the parent directory.
         * @param ent The child entry.
         */
        static std::shared_ptr<dir_type> get(std::shared_ptr<dir_type> dir, const dir_entry &ent);

        /**
         * Returns a directory writer object for creating files in the
         * directory at @a path.
         *
         * @param path Path to the directory.
         *
         * @return The directory writer object.
         */
        static dir_writer *get_writer(const pathname &path);

        /**
         * Virtual file system type.
         */
        enum fs_type {
            /* No type */
            fs_type_none = 0,
            /* Regular directory */
            fs_type_dir,
            /* Virtual file system such as archives. */
            fs_type_virtual
        };

        /**
         * Checks whether two directories are on the same file system.
         *
         * @param dir1 Path to the first directory.
         * @param dir2 Path to the second directory.
         *
         * @return If the two paths are on the same file system
         *   returns the type of the file system (as an fs_type
         *   constant) otherwise returns fs_type_none.
         */
        static fs_type on_same_fs(const pathname::string &dir1, const pathname::string &dir2);

        /**
         * Retrieves the subpath component, within a virtual file
         * system, of a path. Returns the empty string if the path is
         * not within a virtual file system.
         *
         * @param path The path.
         * @return The subpath component.
         */
        static pathname get_subpath(const pathname &path);
    };
}


#endif // NUC_DIRECTORY_DIR_TYPE_H


/* Local Variables: */
/* mode: c++ */
/* indent-tabs-mode: nil */
/* End: */
