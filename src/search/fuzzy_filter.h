/*
 * NuCommander
 * Copyright (C) 2019  Alexander Gutev <alex.gutev@gmail.com>
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

#ifndef SEARCH_FUZZY_FILTER_H
#define SEARCH_FUZZY_FILTER_H

#include <algorithm>

namespace nuc {
    /**
     * Checks whether the string @a string matches the search string
     * @a key, using a fuzzy match.
     *
     * The fuzzy match algorithm checks that each character in @a key
     * occurs somewhere in @a string after the position at which the
     * previous character of @a key occurs in @a string.
     *
     * @param string The string to match.
     * @param key The search string key.
     *
     * @return True if @a key fuzzy matches @a string.
     */
    template <typename T>
    bool fuzzy_match(const T& string, const T& key);
}  // nuc


/// Implementation

template <typename T>
bool nuc::fuzzy_match(const T& string, const T& key) {
    auto sit = string.begin(), send = string.end();
    auto kit = key.begin(), kend = key.end();

    while (kit != kend && sit != send) {
        gunichar kc = g_unichar_toupper(*kit);

        sit = std::find_if(sit, send, [&] (gunichar c) {
            return g_unichar_toupper(c) == kc;
        });

        if (sit != send) {
            ++kit;
            ++sit;
        }
    }

    return kit == kend;
}


#endif /* SEARCH_FUZZY_FILTER_H */

// Local Variables:
// mode: c++
// End:
