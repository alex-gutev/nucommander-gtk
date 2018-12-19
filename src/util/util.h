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

#ifndef NUC_UTIL_UTIL_H
#define NUC_UTIL_UTIL_H

#include <utility>

/**
 * Miscellaneous Utilities
 */

namespace nuc {
    /**
     * Identity Functor
     */
    struct identity {
        /**
         * Returns the object it received as an argument.
         */
        template<typename T>
        constexpr auto operator()(T&& x) const noexcept
            -> decltype(std::forward<T>(x)) {
            return std::forward<T>(x);
        }
    };
}

#endif

// Local Variables:
// mode: c++
// End:
