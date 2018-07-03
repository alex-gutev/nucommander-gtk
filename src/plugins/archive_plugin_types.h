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

/**
 * Archive entry metadata.
 */
typedef struct nuc_arch_entry {
    /** Subpath of the entry within the archive */
    const char *path;

    /**
     * Destination path of hardlink, NULL if the entry is not a
     * hardlink.
     */
    const char *link_dest;
    /**
     * Destination path of symbolic link, NULL if the entry is not a
     * symbolic link.
     */
    const char *symlink_dest;

    /** Stat attributes */
    const struct stat *stat;
} nuc_arch_entry;

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

#endif // NUC_ARCHIVE_PLUGIN_TYPES_H

/* Local Variables: */
/* indent-tabs-mode: nil */
/* End: */
