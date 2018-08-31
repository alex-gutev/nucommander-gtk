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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <archive.h>
#include <archive_entry.h>

#include "plugins/archive_plugin_api.h"

#define EXPORT __attribute__((visibility("default")))

/**
 * Buffer size used to "fill" holes when packing sparse files.
 */
#define HOLE_BUF_SIZE 131072


/**
 * Archive Handle.
 */
typedef struct nuc_arch_handle {
    /**
     *  Libarchive handle and entry.
     */
    struct archive *ar;
    struct archive_entry *ent;

    /**
     * Mode for which the archive was opened.
     */
    nuc_arch_open_mode mode;
    /**
     * File name of open archive (only used if mode ==
     * NUC_AP_MODE_PACK).
     */
    char *dest_file;

    /**
     * Callback function and context pointer.
     */
    nuc_arch_progress_fn callback;
    void *ctx;
} nuc_arch_handle;


//// Function Prototypes

/**
 * Open archive for unpacking (reading).
 *
 * @param file Path to the archive file to open.
 * @param handle Pointer to the handle to initialize.
 *
 * @return Zero (NUC_AP_OK) if successful, a non-zero error constant
 *    if an error occurred.
 */
static int open_unpack(const char *file, nuc_arch_handle *handle);
/**
 * Open archive for packing (writing).
 *
 * @param file Path to the archive file to open.
 * @param handle Pointer to the handle to initialize.
 *
 * @return Zero (NUC_AP_OK) if successful, a non-zero error constant
 *    if an error occurred.
 */
static int open_pack(const char *file, nuc_arch_handle *handle);

/**
 * Close archive opened for unpacking.
 *
 * @param handle Handle to the open archive.
 *
 * @return Zero (NUC_AP_OK) if successful, a non-zero error constant
 *    if an error occurred.
 */
static int close_unpack(nuc_arch_handle *);
/**
 * Close archive opened for unpacking.
 *
 * @param handle Handle to the open archive.
 *
 * @return Zero (NUC_AP_OK) if successful, a non-zero error constant
 *    if an error occurred.
 */
static int close_pack(nuc_arch_handle *);

/**
 * Writes a block of zeroes to the archive.
 *
 * @param handle Handle to archive (open for packing).
 *
 * @return Zero (NUC_AP_OK) if successful, a non-zero error constant
 *    if an error occurred.
 */
static int write_hole(nuc_arch_handle *, size_t len);

/**
 * Calls the progress callback
 */
static nuc_arch_prog_act call_callback(nuc_arch_handle *handle, nuc_arch_prog_type type, int error, size_t bytes);

/**
 * Converts a libarchive error constant to the corresponding NUC_AP_
 * error constant. Sets errno to archive_errno if err is less than
 * zero (not ARCHIVE_OK or ARCHIVE_EOF).
 *
 * @param handle Handle to the archive.
 * @param err The libarchive error constant.
 *
 * @return The corresponding NUC_AP_ error constant.
 */
static int err_code(const nuc_arch_handle *handle, int err);


//// Opening Archives

EXPORT
void *nuc_arch_open(const char *file, int mode, int *error) {
    nuc_arch_handle *handle = malloc(sizeof(nuc_arch_handle));

    if (!handle) {
        *error = NUC_AP_FATAL;
        return NULL;
    }

    handle->callback = NULL;
    handle->dest_file = NULL;
    handle->mode = mode;

    switch (mode) {
    case NUC_AP_MODE_UNPACK:
        *error = open_unpack(file, handle);
        break;

    case NUC_AP_MODE_PACK:
        *error = open_pack(file, handle);
    }

    if (*error) {
        free(handle);
        return NULL;
    }

    return handle;
}

int open_unpack(const char *file, nuc_arch_handle *handle) {
    int err;

    if (!(handle->ar = archive_read_new())) {
        return NUC_AP_FATAL;
    }

    if ((err = archive_read_support_filter_all(handle->ar)) != ARCHIVE_OK) {
        goto cleanup;
    }

    if ((err = archive_read_support_format_all(handle->ar)) != ARCHIVE_OK) {
        goto cleanup;
    }

    if ((err = archive_read_open_filename(handle->ar, file, 10240)) != ARCHIVE_OK) {
        goto cleanup;
    }

    return NUC_AP_OK;

cleanup:
    err = err_code(handle, err);
    archive_read_free(handle->ar);

    return err;
}

int open_pack(const char *file, nuc_arch_handle *handle) {
    int len = strlen(file);

    if (!(handle->dest_file = malloc(len)))
        return NUC_AP_FATAL;

    memcpy(handle->dest_file, file, len + 1);

    if (!(handle->ar = archive_write_new())) {
        free(handle->dest_file);
        return NUC_AP_FATAL;
    }

    return NUC_AP_OK;
}


//// Closing Archives

EXPORT
int nuc_arch_close(void *ctx) {
    nuc_arch_handle *handle = ctx;
    int err = NUC_AP_OK;

    switch (handle->mode) {
    case NUC_AP_MODE_UNPACK:
        err = close_unpack(handle);
        break;

    case NUC_AP_MODE_PACK:
        err = close_pack(handle);
        break;
    };

    free(handle);
    return err;
}

int close_unpack(nuc_arch_handle *handle) {
    int err = err_code(handle, archive_read_close(handle->ar));
    archive_read_free(handle->ar);

    return err;
}

int close_pack(nuc_arch_handle *handle) {
    int err = err_code(handle, archive_write_close(handle->ar));

    free(handle->dest_file);
    archive_write_free(handle->ar);

    return err;
}


//// Set Callback

EXPORT
void nuc_arch_set_callback(void *plg_ctx, nuc_arch_progress_fn callback, void *cb_ctx) {
    nuc_arch_handle *handle = plg_ctx;

    handle->callback = callback;
    handle->ctx = cb_ctx;
}


//// Reading and Unpacking Entries

EXPORT
int nuc_arch_next_entry(void *ctx, nuc_arch_entry *ent) {
    int err;
    nuc_arch_handle *handle = ctx;

    err = err_code(handle, archive_read_next_header(handle->ar, &handle->ent));

    if (err) return err;

    ent->path = archive_entry_pathname(handle->ent);

    ent->link_dest = archive_entry_hardlink(handle->ent);
    ent->symlink_dest = archive_entry_symlink(handle->ent);

    ent->stat = archive_entry_stat(handle->ent);

    return NUC_AP_OK;
}

EXPORT
int nuc_arch_unpack(void *ctx, const char **buf, size_t *size, off_t *offset) {
    nuc_arch_handle *handle = ctx;

    return err_code(handle, archive_read_data_block(handle->ar, (const void **)buf, size, offset));
}


//// Copying archive types and entries

EXPORT
int nuc_arch_copy_archive_type(void *dest_handle, const void *src_handle) {
    const nuc_arch_handle *src = src_handle;
    nuc_arch_handle *dest = dest_handle;

    int num_filters = archive_filter_count(src->ar);
    int err;

    for (int i = 0; i < num_filters; i++) {
        err = err_code(dest, archive_write_add_filter(dest->ar, archive_filter_code(src->ar, i)));
        if (err) return err;
    }

    if ((err = err_code(dest, archive_write_set_format(dest->ar, archive_format(src->ar))))) {
        return err;
    }

    return err_code(dest, archive_write_open_filename(dest->ar, dest->dest_file));
}


EXPORT
int nuc_arch_copy_last_entry(void *dest_handle, const void *src_handle) {
    const nuc_arch_handle *src = src_handle;
    nuc_arch_handle *dest = dest_handle;

    int err;

    if ((err = err_code(dest, archive_write_header(dest->ar, src->ent)))) {
        return err;
    }

    const void *buf;
    size_t size;
    off_t offset, last_offset = 0;

    while (!(err = err_code(src, archive_read_data_block(src->ar, &buf, &size, &offset)))) {

        if (offset - last_offset) {
            if ((err = write_hole(dest, offset - last_offset))) {
                return err;
            }
        }

        if (archive_write_data(dest->ar, buf, size) < 0) {
            errno = archive_errno(dest->ar);
            return NUC_AP_FAILED;
        }

        last_offset = offset + size;
    }

    return err = NUC_AP_EOF ? NUC_AP_OK : err;
}


EXPORT
int nuc_arch_create_entry(void *ctx, const nuc_arch_entry *ent) {
    nuc_arch_handle *handle = ctx;
    struct archive_entry *ar_ent = archive_entry_new();

    int err;

    if (!ar_ent) return NUC_AP_FATAL;

    archive_entry_set_pathname(ar_ent, ent->path);
    archive_entry_copy_stat(ar_ent, ent->stat);

    err = err_code(handle, archive_write_header(handle->ar, ar_ent));

    archive_entry_free(ar_ent);

    return err;
}

EXPORT
int nuc_arch_pack(void *ctx, const char *buf, size_t len, off_t offset) {
    nuc_arch_handle *handle = ctx;

    if (offset) {
        int err = write_hole(handle, offset);
        if (err) return err;
    }

    if (archive_write_data(handle->ar, buf, len) < 0) {
        errno = archive_errno(handle->ar);
        return NUC_AP_FAILED;
    }

    return NUC_AP_OK;
}


static int write_hole(nuc_arch_handle *handle, size_t len) {
    size_t buf_size = len < HOLE_BUF_SIZE ? len : HOLE_BUF_SIZE;
    char *zeros = calloc(len, buf_size);

    if (!zeros)
        return NUC_AP_FATAL;

    while (len) {
        if (archive_write_data(handle->ar, zeros, len < buf_size ? buf_size : len) < 0) {
            errno = archive_errno(handle->ar);
            return NUC_AP_FAILED;
        }

        len -= buf_size;
    }

    return NUC_AP_OK;
}

//// Utility Functions

nuc_arch_prog_act call_callback(nuc_arch_handle *handle, nuc_arch_prog_type type, int error, size_t bytes) {
    if (handle->callback) {
        return handle->callback(handle->ctx, type, error, bytes);
    }

    return error ? NUC_AP_ACT_ABORT : NUC_AP_ACT_CONTINUE;
}


static int err_code(const nuc_arch_handle *handle, int err) {
    if (err < 0)
        errno = archive_errno(handle->ar);

    switch (err) {
        case ARCHIVE_OK:
            return NUC_AP_OK;

        case ARCHIVE_EOF:
            return NUC_AP_EOF;

        case ARCHIVE_RETRY:
            return NUC_AP_RETRY;

        case ARCHIVE_WARN:
            return NUC_AP_WARN;

        case ARCHIVE_FAILED:
            return NUC_AP_FAILED;

        case ARCHIVE_FATAL:
            return NUC_AP_FATAL;
    }

    return NUC_AP_FATAL;
}


/* Local Variables: */
/* indent-tabs-mode: nil */
/* End: */
