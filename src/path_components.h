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

#ifndef NUC_PATH_COMPONENTS_H
#define NUC_PATH_COMPONENTS_H

#include <string>
#include <vector>

#include "types.h"

namespace nuc {
    /**
     * Splits a path string into its components.
     */
    class path_components {
        /** Reference to the path string. */
        const path_str &path;
        
    public:

        /**
         * Path component iterator.
         */
        class iter {
            /** Reference to the path string. */
            const path_str &path;

            /**
             * Index of the first character of the current component.
             */
            size_t pos = 0;
            /**
             * Index of the next '/' character, or past the end of
             * string index.
             */
            size_t next_pos = 0;

            /**
             * Returns the index of the '/' next slash in the path
             * string.
             */
            size_t next_slash();

            /**
             * Returns the index of the first non-slash character
             * after 'next_pos'.
             */
            size_t next_non_slash() const;
            
            /**
             * Advances the current position to the first non-slash
             * character after 'next_pos', stops at the end of string.
             */
            void next();
            
        public:
            /**
             * Creates an iterator with a given path and index.
             */
            iter(const path_str &path, size_t pos);

            /**
             * Returns the substring of the path upto and including
             * the current component.
             */
            path_str sub_path() const {
                return path.substr(0, next_pos);
            }

            /**
             * Returns the position of the first character of the
             * component within the path string.
             */
            size_t position() const {
                return pos;
            }

            /**
             * Returns true if this component, is the last component
             * in the path.
             */
            bool last() const {
                return next_non_slash() == path_str::npos;
            }
            
            iter &operator++() {
                next();
            }
            iter operator++(int) {
                iter copy = iter(*this);
                next();
                
                return copy;
            }

            /**
             * Returns the current component string.
             */
            path_str operator*() const;
            
            bool operator==(const iter &it) const {
                return pos == it.pos;
            }
            bool operator!=(const iter &it) const {
                return pos != it.pos;
            }
        };

        /**
         * Constructs a 'path_component' object. The only purpose of
         * this object is to obtain iterators to the path components.
         *
         * path: The path string. The string is not copied, a
         *       reference to it is stored instead, thus should be
         *       kept in memory whilst this object is used.
         */
        path_components(const path_str &path) : path(path) {}

        /**
         * Returns an array of all path components in the path string.
         */
        static std::vector<path_str> all(const path_str &path);

        /** Iterators */
        iter begin();
        iter end();
    };
}

#endif // NUC_PATH_COMPONENTS_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
