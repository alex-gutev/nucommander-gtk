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

#ifndef NUC_DIR_LISTER_H
#define NUC_DIR_LISTER_H

#include "lister/lister.h"

namespace nuc {

    /**
     * Implements the lister interface for reading regular on disk
     * directories.
     */
    class dir_lister : public lister {
        /** Directory handle. */
        DIR *dp = nullptr;

        /** Last entry read. */
        struct dirent *last_ent;

        /**
         * Returns the next entry or nullptr if there are no more
         * entries. If an error occurs, an 'error' exception is thrown
         * with the value of 'errno'.
         */
        struct dirent *next_ent();

    public:
        /**
         * Destructor. Closes the directory handle, if open.
         */
        virtual ~dir_lister();

        /** Method Overrides */

        virtual void open(const paths::string &path);

        virtual void close();

        virtual bool read_entry(entry &ent);
        virtual bool entry_stat(struct stat &st);
    };
}

#endif // NUC_DIR_LISTER_H

// Local Variables:
// mode: c++
// End:
