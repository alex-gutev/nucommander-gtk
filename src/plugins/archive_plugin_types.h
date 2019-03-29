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

#ifndef NUC_ARCHIVE_PLUGIN_TYPES_H
#define NUC_ARCHIVE_PLUGIN_TYPES_H

#include <sys/stat.h>
#include <stdlib.h>

/**
 * Contains types and constants related to archive plugins.
 */

/**
 * Archive open mode.
 */
typedef enum {
    /**
     * Open an archive for unpacking and/or listing its contents.
     */
    NUC_AP_MODE_UNPACK = 0,
    /**
     * Create an archive to which files will be added.
     */
    NUC_AP_MODE_PACK,
} nuc_arch_open_mode;

/**
 * Archive plugin error type constants.
 */
typedef enum {
    /** The last operation may be retried */
    NUC_AP_RETRY = -1,
    /** Last operation completed with errors */
    NUC_AP_WARN = -2,
    /** Last operation failed and cannot be retried */
    NUC_AP_FAILED = -3,
    /**
     * Last operation failed and no more operations can be attempted
     * on the current archive handle.
     */
    NUC_AP_FATAL = -4,

    /** No Error */
    NUC_AP_OK = 0,
    /** End of file or last entry read */
    NUC_AP_EOF = 1,
} nuc_arch_plugin_error;


/**
 * Unpacking types.
 */

/** (Un)packing operation stage */
typedef enum {
    /**
     * Beginning (un)packing.
     *
     * The callback is first called with this stage.
     */
    NUC_AP_BEGIN = 0,
    /**
     * (Un)packing in progress.
     *
     * The bytes parameter contains the number of bytes processed.
     */
    NUC_AP_PROGRESS,
    /**
     * (Un)packing finished.
     *
     * The callback is called with this stage after finishing.
     */
    NUC_AP_FINISH
} nuc_arch_prog_type;

/**
 * Progress callback action.
 *
 * Action to take after returning from the progress callback.
 */
typedef enum {
    /**
     * Abort the operation.
     */
    NUC_AP_ACT_ABORT = 0,
    /**
     * Continue with the operation. Retry in the case of error.
     */
    NUC_AP_ACT_CONTINUE,
} nuc_arch_prog_act;

/**
 * Progress callback.
 *
 * @param ctx   Context pointer, provided in nuc_arch_set_callback.
 * @param type  Progress type.
 * @param error 0 - No error, otherwise the error code of the error.
 *
 * @param bytes Number of bytes processed since the last callback,
 *    only used if the progress typ is NUC_AP_UNPACK_PROGRESS.
 *
 * @return The action to take after returning, either continue with
 *    the operation (NUC_AP_ACT_CONTINUE), or abort
 *    (NUC_AP_ACT_ABORT).
 */
typedef nuc_arch_prog_act(*nuc_arch_progress_fn)(void *ctx, nuc_arch_prog_type type, int error, size_t bytes);

/**
 * Read callback function type.
 *
 * Called by the plugin to read the next block of data of the archive
 * file.
 *
 * @param ctx Context pointer.
 *
 * @param buffer Pointer to a pointer which should be set to point to
 *   the block just read.
 *
 * @return The size of the block read. 0 if the end of file was
 *   reached (no data was read). -1 if an error occurred.
 */
typedef ssize_t(*nuc_arch_read_callback)(void *ctx, const void **buffer);
/**
 * Skip callback function type.
 *
 * Called by the plugin to skip the next block of bytes.
 *
 * @param ctx Context pointer.
 * @param n The number of bytes to skip.
 *
 * @return The number of bytes actually skipped, which may be less
 *   than the @a n.
 */
typedef off_t(*nuc_arch_skip_callback)(void *ctx, off_t n);

/**
 * Write callback function type.
 *
 * Called by the plugin to write a block of data to the medium in
 * which the archive is stored.
 *
 * @param ctx Context pointer.
 *
 * @param buffer Pointer to the block of data to write.
 *
 * @param length The size of the buffer.
 *
 * @return The number of bytes actually written or -1 if an error
 *   occurred.
 */
typedef ssize_t(*nuc_arch_write_callback)(void *ctx, const void *buffer, size_t length);

#endif // NUC_ARCHIVE_PLUGIN_TYPES_H

/* Local Variables: */
/* indent-tabs-mode: nil */
/* End: */
