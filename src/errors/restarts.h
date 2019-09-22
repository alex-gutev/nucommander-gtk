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

    /**
     * Skip attribute exception.
     *
     * This exception is thrown when the "skip attribute" is invoked,
     * to skip setting the attribute which triggered the error.
     */
    class skip_attribute : public std::exception {};


    /* Utilities */

    /**
     * Executes the function op with the "skip attribute" restart
     * established,
     *
     * @param op The function to execute.
     */
    template <typename F>
    void with_skip_attrib(F op);
}


/* Template Implementations */

template <typename F>
void nuc::with_skip_attrib(F op) {
    global_restart skip(restart("skip attribute", [] (const error &, boost::any) {
        throw skip_attribute();
    }));

    try {
        op();
    }
    catch (const skip_attribute &) {
    }
}

#endif

// Local Variables:
// mode: c++
// End:
