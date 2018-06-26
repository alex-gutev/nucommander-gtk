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

#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <string>
#include <vector>

#include "types.h"
#include "path_components.h"

/**
 * Path utility functions.
 */

namespace nuc {
    /** Retrieving various parts */
    
    /**
     * Returns the file name (basename) of 'path'.
     */
    path_str file_name(const path_str &path);
    /**
     * Returns the file extension of the file in 'path'.
     */
    path_str file_extension(const path_str &path);

    
    /** Path components */
    
    /**
     * Appends the component 'comp' to the path string 'path'. If
     * 'path' does not end in a '/', and it is not an empty string,
     * a '/' is inserted before the component, otherwise it is not
     * inserted.
     */
    void append_component(path_str &path, const path_str &comp);

    /**
     * Returns a new path string with a component appended to an
     * existing path.
     * 
     * path: The exisitng path string, on which to append the
     *       component.
     *
     * comp: The component to append.
     * 
     * Returns the new path string.
     */
    path_str appended_component(path_str path, const path_str &comp);
    
    /**
     * Creates a path string from an array of path components.
     */
    path_str path_from_components(const std::vector<path_str> &comps);

    /**
     * Removes the last path component of the path string 'path'.
     */
    void remove_last_component(path_str &path);
    
    /**
     * Returns a new string which is a copy of 'path' with the last
     * component removed.
     */
    path_str removed_last_component(path_str path);
    
    
    /**
     * Returns the canonical representation of a path by removing all
     * '.', '..' components and double slashes. Leading '..'
     * components or '..' components which refer to a parent directory
     * of the base directory in 'path', e.g. /foo/../../bar, are not
     * removed.
     */
    path_str canonicalized_path(const path_str &path);


    /**
     * Returns true if 'path' is the file system root.
     */
    bool is_root_path(const path_str &path);

    /**
     * Checks whether @a path is a direct child of @a dir.
     *
     * @param dir The parent directory
     * @param path The path to test whether it is a child of @a dir
     *
     * @return True if @a path is a subpath of @a dir and its last
     *    component is a child of @a dir.
     */
    bool is_child_of(path_str dir, const path_str &path);

    /**
     * Checks whether a path has any directory components.
     * 
     * @param path The path to test
     *
     * @return True if the path has directory components, false
     *    otherwise.
     */
    inline bool has_dirs(const path_str &path) {
        return path.find('/') != std::string::npos;
    }

    /**
     * Checks whether a path is a relative path. Paths with a leading
     * tilde '~' are considered absolute paths.
     *
     * @param path The path to check.
     *
     * @return True if @a path is relative.
     */
    inline bool is_relative_path(const path_str &path) {
        if (!path.empty()) {
            return path.front() != '/' && path.front() != '~';
        }

        return true;
    }

    /**
     * Expands the leading tilde '~' in a path to the user's home
     * directory. If the tilde cannot be expanded (the user doesn't
     * exist) then the path is returned as is.
     *
     * @param path The path to expand.
     *
     * @return The path with the leading tilde expanded.
     */
    path_str expand_tilde(const path_str &path);
}

#endif // PATH_UTILS_H

// Local Variables:
// mode: c++
// End:
