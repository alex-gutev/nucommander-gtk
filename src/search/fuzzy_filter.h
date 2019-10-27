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
#include <tuple>

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
     * @return A pair of:
     *   - A Boolean which is true if @a key fuzzy matches @a string.
     *
     *   - An accuracy score, the lower the score the more closely @a
     *     key matches @a string.
     */
    template <typename T>
    std::pair<bool, float> fuzzy_match(const T& string, const T& key);
}  // nuc


//// Implementation

template <typename T>
std::pair<bool, float> nuc::fuzzy_match(const T& string, const T& key) {
    auto sit = string.begin(), send = string.end();
    auto kit = key.begin(), kend = key.end();

    size_t start = T::npos;

    while (kit != kend && sit != send) {
        gunichar kc = g_unichar_toupper(*kit);

        sit = std::find_if(sit, send, [&] (gunichar c) {
            return g_unichar_toupper(c) == kc;
        });

        if (sit != send) {
            if (start == T::npos)
                start = std::distance(string.begin(), sit);

            ++kit;
            ++sit;
        }
    }

    if (kit == kend) {
        size_t end = std::distance(string.begin(), sit);

        return std::make_pair(true,  key.length() / float(end - start) * (1 - float(start) / string.length()));
    }

    return std::make_pair(false, 0);
}


#endif /* SEARCH_FUZZY_FILTER_H */

// Local Variables:
// mode: c++
// End:
