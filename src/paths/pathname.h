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

#ifndef NUC_PATHS_PATHNAME_H
#define NUC_PATHS_PATHNAME_H

#include <string>
#include <vector>
#include <set>

/**
 * Path utility functions.
 */

namespace nuc {
    namespace paths {
        /**
         * Path string type.
         */
        typedef std::string string;

        /**
         * Pathname class.
         *
         * Provides a wrapper around a path string and some functions
         * for manipulating paths.
         */
        class pathname {
            /**
             * The path string.
             */
            string m_path;

            /**
             * Ensures that the path contains/does not contain a
             * trailing slash.
             *
             * @param is_dir True if the path should contain a
             *   trailing slash, false if it should not.
             */
            void ensure_trail_slash(bool is_dir);

        public:
            /**
             * Constructs an empty pathname.
             */
            pathname() : pathname("", false) {}

            /**
             * Constructs a pathname for a given path string.
             *
             * @param path The path string.
             */
            template <typename T>
            pathname(T&& path) : m_path(std::forward<T>(path)) {}

            /**
             * Constructs a pathname for a given string.
             *
             * @param path The path string.
             *
             * @param is_dir True if the path represents a directory
             *   and should have a trailing slash, false otherwise.
             */
            template <typename T>
            pathname(T&& path, bool is_dir) : m_path(std::forward<T>(path)) {
                ensure_trail_slash(is_dir);
            }

            /**
             * Creates a path from an array of path components.
             *
             * @param components Array of path components.
             *
             * @param is_dir True if the path represents a directory
             *   and should thus have a trailing slash added to it.
             */
            static pathname from_components(const std::vector<string> &components, bool is_dir = false);


            /* Path string, components and properties accessors */

            /**
             * Returns the path string.
             *
             * @return The path string.
             */
            const string &path() const {
                return m_path;
            }

            /**
             * Returns true if the path represents a directory (has a
             * trailing slash).
             *
             * @return True if the path represents a directory false
             * otherwise.
             */
            bool is_dir() const;

            /**
             * Splits the path into its components. Empty components
             * are removed.
             *
             * @return Array of path components.
             */
            std::vector<string> components() const;


            /* Manipulating Paths */

            /**
             * Creates a new pathname with the path @a path appended
             * to it.
             *
             * @param path The path to append.
             * @return The new pathname.
             */
            pathname append(const pathname &path) const &;
            pathname& append(const pathname &path) &&;

            /**
             * Creates a new pathname with the last component removed.
             *
             * @return The new pathname.
             */
            pathname remove_last_component() const &;
            pathname& remove_last_component() &&;

            /**
             * Creates a new pathname which is a copy of this merged
             * with @a path.
             *
             * If @a path is a relative path, the file component of
             * this is removed (if it does not represent a directory)
             * and @a path is appended to it. If @a path is an
             * absolute path it is returned.
             *
             * @param path The path to merge into this.
             * @return The new pathname.
             */
            pathname merge(const pathname &path) const &;
            pathname& merge(const pathname &path) &&;

            /**
             * Creates a new pathname which is the canonical
             * representation of this. All '.' and '..' entries are
             * removed.
             *
             * @param is_dir True if the new path should contain a
             *   trailing slash, false otherwise.
             *
             * @return The new pathname.
             */
            pathname canonicalize(bool is_dir = false) const &;
            pathname& canonicalize(bool is_dir = false) &&;

            /**
             * Creates a new path with the leading '~', if any,
             * expanded to the user's home directory.
             *
             * @return The new pathname.
             */
            pathname expand_tilde() const;


            /* Retrieving Last Component. */

            /**
             * Returns the last component of the pathname.
             *
             * If the path has a trailing slash the component
             * preceding the trailing slash is returned.
             *
             * If the path is the file system root '/', it is
             * returned.
             *
             * @return The last component.
             */
            string basename() const;
            /**
             * Returns the extension of the basename component if any.
             *
             * @return The extension string, following the '.'
             */
            string extension() const;

            /**
             * Returns the offset of the first character of the
             * basename component.
             *
             * @return The index of the byte making up the first
             *   character of the basename component.
             */
            size_t basename_offset() const;


            /* Querying Path Types */

            /**
             * Returns true if the filesystem root.
             *
             * @return true or false.
             */
            bool is_root() const;
            /**
             * Returns true if the path is a relative path.
             *
             * Paths with leading tilde's are not considered relative paths.
             *
             * @return true or false.
             */
            bool is_relative() const;

            /**
             * Checks whether the path is a subpath of @a parent.
             *
             * @param parent The path to check whether this is a
             *   subpath of.
             *
             * @param check_dir If true, additionally checks that
             *   parent represents a directory.
             *
             * @return True if this is a subpath of @a parent.
             */
            bool is_subpath(const pathname &parent, bool check_dir = false) const;
            /**
             * Checks whether the path is a subpath and direct child
             * of @a parent.
             *
             * @param parent The path to check whether this is a
             *   child of.
             *
             * @return True if this is a child of @a parent.
             */
            bool is_child_of(const pathname &parent) const;

            /**
             * Checks whether the path has any directory components.
             *
             * @param True if the path has any directory components,
             *   false otherwise.
             */
            bool has_dirs() const;

            /**
             * Returns true if the path has no components.
             *
             * @return true or false.
             */
            bool empty() const {
                return m_path.empty();
            }


            /* Cast Operators */

            operator string() const {
                return m_path;
            }

            /**
             * Returns a C string representation of the path.
             *
             * @return C string representation of the path.
             */
            const char *c_str() const {
                return m_path.c_str();
            }


            /* Utility Functions */

            /**
             * Checks whether @path is a subpath of a path in the set
             * @a paths, and returns the offset of the basename
             * component of the path of which @a path is a subpath.
             *
             * @return Offset of the basename component or
             * string::npos if @a path is not a subpath of any path.
             */
            static size_t subpath_offset(const std::set<paths::pathname> &paths, const pathname &path);
        };

        /* Comparison Operators */

        inline bool operator == (const pathname &p1, const pathname &p2) {
            return p1.path() == p2.path();
        }
        inline bool operator != (const pathname &p1, const pathname &p2) {
            return p1.path() != p2.path();
        }
        inline bool operator < (const pathname &p1, const pathname &p2) {
            return p1.path() < p2.path();
        }
        inline bool operator <= (const pathname &p1, const pathname &p2) {
            return p1.path() <= p2.path();
        }
        inline bool operator > (const pathname &p1, const pathname &p2) {
            return p1.path() > p2.path();
        }
        inline bool operator >= (const pathname &p1, const pathname &p2) {
            return p1.path() >= p2.path();
        }
    }
}

/* Hash Function */

namespace std {
    template <> struct hash<nuc::paths::pathname> {
        size_t operator()(const nuc::paths::pathname &path) const {
            return hash<nuc::paths::string>()(path);
        }
    };
}

#endif // NUC_PATHS_UTILS_H

// Local Variables:
// mode: c++
// End:
