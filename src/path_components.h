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

namespace nuc {
    class path_components {
        const std::string &path;
        
    public:
        
        class iter {
            const std::string &path;
            
            size_t pos = 0;
            size_t next_pos = 0;
            
        public:
            
            iter(const std::string &path, size_t pos);
            
            size_t next_slash();
            void next();
            
            std::string sub_path() const {
                return path.substr(0, next_pos);
            }
            
            bool last() const {
                return next_pos == path.length();
            }
            
            
            iter &operator++() {
                next();
            }
            iter operator++(int) {
                iter copy = iter(*this);
                next();
                
                return copy;
            }
            
            std::string operator*() const;
            
            bool operator==(const iter &it) const {
                return pos == it.pos;
            }
            bool operator!=(const iter &it) const {
                return pos != it.pos;
            }
        };
        
        path_components(const std::string &path) : path(path) {}
        
        static std::vector<std::string> all(const std::string &path);
        
        iter begin();
        iter end();
    };
}

#endif // NUC_PATH_COMPONENTS_H
