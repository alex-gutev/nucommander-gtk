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

#ifndef NUC_ERRORS_RESTARTS_H
#define NUC_ERRORS_RESTARTS_H

#include "error.h"

namespace nuc {
    /**
     * Skip exception.
     *
     * This exception is thrown when the "skip" restart is invoked,
     * and is caught further up the call stack to skip processing the
     * current file.
     */
    struct skip_exception {
        /**
         * Skip restart function. Throws a "skip_exception".
         */
        static void skip(const error &, boost::any) {
            throw skip_exception();
        }

        /**
         * The skip restart.
         */
        static const nuc::restart restart;
    };
}

#endif

// Local Variables:
// mode: c++
// End:
