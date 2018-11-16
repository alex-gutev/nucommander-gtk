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

#ifndef NUC_ERRORS_ATTRIBUTE_ERROR_H
#define NUC_ERRORS_ATTRIBUTE_ERROR_H

#include "file_error.h"

#include "paths/utils.h"

#include <utility>

namespace nuc {
    /**
     * Error involving a particular file.
     */
    class attribute_error : public file_error {
    public:
        /**
         * Constructor.
         *
         * @param code The error code.
         *
         * @param can_retry Flag: true if the operation can be
         *   retried.
         *
         * @param file Path to the file which could not be created.
         */
        template <typename T>
        attribute_error(int code, type_code type, bool can_retry, T&& file) : nuc::file_error(code, type, can_retry, file) {}

        virtual Glib::ustring type_explanation() const noexcept;
    };
}

#endif

// Local Variables:
// mode: c++
// End:
