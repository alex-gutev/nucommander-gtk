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

#ifndef NUC_FILE_LIST_DIRECTORY_BUFFERS_H
#define NUC_FILE_LIST_DIRECTORY_BUFFERS_H

#include <unordered_set>
#include <memory>

#include "file_list_controller.h"

namespace nuc {
    /**
     * Manages the set of file_list_controller's for all the open
     * directories.
     */
    class directory_buffers {
    public:
        /**
         * Directory buffer set type.
         */
        typedef std::unordered_set<std::shared_ptr<file_list_controller>> buffer_set;

        /**
         * Returns the singleton instance.
         *
         * @return directory_buffers&
         */
        static directory_buffers &instance();

        /**
         * Returns the set of file_list_controller's of all the open
         * directories.
         *
         * @return buffer_set&
         */
        buffer_set &buffers();

        /**
         * Creates a new file_list_controller.
         *
         * @return Shared pointer to the file_list_controller.
         */
        std::shared_ptr<file_list_controller> new_buffer();

        /**
         * Removes a file_list_controller from the set.
         *
         * @param flist The file_list_controller to remove.
         */
        void close_buffer(std::shared_ptr<file_list_controller> flist);

    private:
        /** Set of file_list_controllers. */
        buffer_set bufs;
    };

}  // nuc


#endif /* NUC_FILE_LIST_DIRECTORY_BUFFERS_H */

// Local Variables:
// mode: c++
// End:
