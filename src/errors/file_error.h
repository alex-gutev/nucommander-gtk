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

#ifndef NUC_ERRORS_CREATE_ERROR_H
#define NUC_ERRORS_CREATE_ERROR_H

#include "error.h"

#include "paths/pathname.h"

#include <utility>

namespace nuc {
    /**
     * Error involving a particular file.
     */
    class file_error : public error {
    protected:
        /**
         * The name of the file which triggered the error.
         */
        paths::string file;

    public:
        using error::error;

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
        file_error(int code, type_code type, bool can_retry, T&& file) :
            nuc::error(code, type, can_retry), file(std::forward<T>(file)) {}

        virtual Glib::ustring type_explanation() const noexcept;
    };
}

#endif

// Local Variables:
// mode: c++
// End:
