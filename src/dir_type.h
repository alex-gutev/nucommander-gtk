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

#ifndef NUC_DIR_TYPE_H
#define NUC_DIR_TYPE_H

#include <tuple>
#include <functional>

#include "types.h"
#include "lister.h"
#include "dir_tree.h"
#include "dir_entry.h"

/**
 * Functions for determining the lister and directory tree (dir_tree)
 * objects for a directory.
 */

namespace nuc {
    /**
     * Directory type class.
     *
     * Stores the path of the directory, the subpath within the
     * virtual directory tree and provides functions for creating the
     * lister and directory tree object for the directory type.
     */
    class dir_type {
        /**
         * Create lister function type.
         *
         * Returns a pointer to a newly created lister object.
         */
        typedef std::function<lister*()> create_lister_fn;
        /**
         * Create directory tree function type.
         *
         * Takes as argument the subpath of the tree and returns a
         * newly created dir_tree object with the subpath set to the
         * subpath passed as an argument.
         */
        typedef std::function<dir_tree*(paths::string)> create_tree_fn;

        /**
         * The path of the actual directory file which is read by the
         * lister.
         */
        paths::string m_path;
        
        /**
         * The initial subpath of the directory tree which is created.
         */
        paths::string m_subpath;

        /**
         * Create lister function.
         */
        create_lister_fn m_create_lister;
        /**
         * Create directory tree function.
         */
        create_tree_fn m_create_tree;

        /**
         * Flag indicating whether the directory is a regular
         * directory or file.
         */
        bool m_is_dir = false;


        /**
         * Canonicalizes a path by expanding leading tilde's and
         * removing all '.' and '..' directory components.
         *
         * @param path The path to canonicalize.
         *
         * @return The canonicalized path.
         */
        static paths::string canonicalize(const paths::string &path);

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
        static std::pair<paths::string, paths::string> canonicalize_case(const paths::string &path);

        /**
         * Searches the directory, at @a dir, for an entry with a name
         * that is equal to @a comp or the first entry with a name
         * that matches, ignoring case, @a comp.
         *
         * @return The name of the matching entry or an empty string
         *    if the directory @a dir could not be read.
         */
        static paths::string find_match_comp(const paths::string &dir, const paths::string &comp);

    public:
        /**
         * Constructs an "empty" dir_type object with no path and no
         * creation functions.
         */
        dir_type() : dir_type("", nullptr, nullptr, false, "") {}

        /**
         * Constructs a dir_type object with a given path, subpath and
         * creation functions.
         *
         * @param path          The path to the directory file.
         * @param create_lister Lister creation function.
         * @param create_tree   Tree creation function.
         * @param is_dir        True if directory is a regular directory.
         * @param subpath       Initial subpath of tree.
         */
        dir_type(paths::string path, create_lister_fn create_lister, create_tree_fn create_tree, bool is_dir, paths::string subpath)
            : m_path(std::move(path)), m_subpath(std::move(subpath)),
              m_create_lister(create_lister), m_create_tree(create_tree), m_is_dir(is_dir) {}


        /* Creating lister and dir_tree */
        
        /**
         * Creates the lister object.
         *
         * @return Pointer to a newly created lister object.
         */
        lister * create_lister() const {
            return m_create_lister();
        }

        /**
         * Creates the directory tree, with the subpath set to the
         * subpath stored in the dir_type object.
         *
         * @return Pointer to a newly created dir_tree object.
         */
        dir_tree * create_tree() const {
            return m_create_tree(m_subpath);
        }

        /**
         * @return True if dir_type is not empty.
         */
        explicit operator bool() const {
            return static_cast<bool>(m_create_lister);
        }


        /* Accessors */

        /**
         * @return True if the directory is a regular directory.
         */
        bool is_dir() const {
            return m_is_dir;
        }

        /**
         * @return The path to the directory file.
         */
        const paths::string & path() const {
            return m_path;
        }

        /**
         * @return The initial subpath of the directory tree.
         */
        const paths::string & subpath() const {
            return m_subpath;
        }

        /**
         * Sets the initial subpath of the directory trees to be
         * created.
         *
         * @param subpath The subpath
         */
        void subpath(const paths::string subpath) {
            m_subpath = subpath;
        }


        /**
         * Returns the logical path of the directory. The logical path
         * is the concatenated path of where the directory is located
         * in the filesystem and the subpath within the directory.
         *
         * @return The logical path.
         */
        paths::string logical_path() const {
            return m_subpath.empty() ? m_path : paths::appended_component(m_path, m_subpath);
        }

        /* Determining the type of a directory */

        /**
         * Determines the directory type of the directory @a path.
         *
         * The path @a path is canonicalized before being stored in
         * the dir_type object created, which can later be retrieved
         * using logical_path().
         *
         * @param path Path to the directory.
         */
        static dir_type get(const paths::string &path);

        /**
         * Determines the directory type of the entry @a ent within
         * the directory at path @a path.
         *
         * Does not perform any blocking operations.
         *
         * @param path Path to the directory containing the entry.
         * @param ent  The entry within the directory @a path.
         */
        static dir_type get(paths::string path, const dir_entry &ent);
    };
}


#endif // NUC_DIR_TYPE_H 


/* Local Variables: */
/* mode: c++ */
/* indent-tabs-mode: nil */
/* End: */
