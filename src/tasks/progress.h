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

#ifndef NUC_TASKS_PROGRESS
#define NUC_TASKS_PROGRESS

#include <functional>

#include "paths/pathname.h"

namespace nuc {
    /**
     * Operation Progress Event.
     */
    struct progress_event {
        /**
         * Progress callback function type.
         */
        typedef std::function<void(const progress_event &)> callback;

        /**
         * Event type.
         */
        enum progress_type {
            /* Begin operation */
            type_begin = 0,
            /* Finish operation */
            type_finish,

            /* Begin processing directory */
            type_enter_dir,
            /* Finish processing directory */
            type_exit_dir,

            /* Begin processing file */
            type_enter_file,
            /* Finish processing file */
            type_exit_file,

            /* Processed file data */
            type_process_data,
        };

        /**
         * Event type.
         */
        progress_type type;

        /**
         * Path to the file associated with the event.
         *
         * This is only used if the event type is one of the
         * following:
         *
         * - type_enter_dir
         * - type_exit_dir
         * - type_enter_file
         * - type_exit_file
         */
        paths::pathname file;

        /**
         * If the event type is type_process_data this is the number
         * of bytes processed.
         *
         * If the event type is type_enter_file this is the file size.
         *
         * Otherwise this field is not used.
         */
        size_t bytes;


        /** Constructors */

        progress_event(progress_type type) : type(type) {}

        progress_event(progress_type type, paths::pathname file, size_t bytes = 0) : type(type), file(file), bytes(bytes) {}

        progress_event(progress_type type, size_t bytes) : type(type), bytes(bytes) {}
    };
};

#endif

// Local Variables:
// mode: c++
// End:
