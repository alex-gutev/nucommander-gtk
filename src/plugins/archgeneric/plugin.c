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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
    /**
     * Last entry read, if unpacking.
     *
     * Last entry created, if packing.
     */
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
     * File descriptor of a temporary file to which the current
     * entry's data is written.
     */
    FILE* tmp_file;
    /**
     * Size of the entry currently being written. This is only used if
     * the size of the entry is unknown, and its data was written to a
     * temporary file first.
     */
    size_t ent_size;

    /**
     * Callback function and context pointer.
     */
    nuc_arch_progress_fn callback;
    void *ctx;

    /**
     * Read callback function.
     */
    nuc_arch_read_callback read_fn;
    /**
     * Skip callback function.
     */
    nuc_arch_skip_callback skip_fn;

    /**
     * Write callback function.
     */
    nuc_arch_write_callback write_fn;

    /**
     * Context pointer argument for read, skip and write callbacks.
     */
    void *callback_ctx;
} nuc_arch_handle;


//// Function Prototypes

/**
 * Allocates a new zero-initialized, plugin handle.
 *
 * @return The handle, or NULL if the allocation failed.
 */
static nuc_arch_handle *alloc_handle();


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
 * Open callback function.
 *
 * @param ar Archive handle.
 * @param ctx Plugin handle.
 *
 * @return Always returns ARCHIVE_OK.
 */
int open_callback(struct archive *ar, void *ctx);
/**
 * Open callback function.
 *
 * @param ar Archive handle.
 * @param ctx Plugin handle.
 *
 * @return Always returns ARCHIVE_OK.
 */
int close_callback(struct archive *ar, void *ctx);
/**
 * Read callback function.
 *
 * Calls the read callback function provided to the plugin in
 * nuc_arch_open_unpack.
 *
 * @param ar Archive handle.
 *
 * @param ctx Plugin handle.
 *
 * @param buffer Pointer to pointer which is set to point to the block
 *   of data read.
 *
 * @return The number of bytes read, 0 on EOF or -1 on error.
 */
ssize_t read_callback(struct archive *ar, void *ctx, const void **buffer);
/**
 * Skip callback function.
 *
 * Calls the skip callback function provided to the plugin in
 * nuc_arch_open_unpack.
 *
 * @param ar Archive handle.
 * @param ctx Plugin handle.
 * @param request The number of bytes to skip.
 *
 * @return The number of bytes actually skipped.
 */
off_t skip_callback(struct archive *ar, void *ctx, off_t request);

/**
 * Creates a new archive_entry, and stores it in the ent member of the
 * handle. If ent contains another entry it is freed first.
 *
 * Should only be called when packing.
 *
 * @param handle Archive handle.
 */
static void create_entry(nuc_arch_handle *handle);

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

static nuc_arch_handle *alloc_handle() {
    return calloc(1, sizeof(nuc_arch_handle));
}

EXPORT
void *nuc_arch_open(const char *file, int mode, int *error) {
    nuc_arch_handle *handle = alloc_handle();

    if (!handle) {
        *error = NUC_AP_FATAL;
        return NULL;
    }

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
    if (!(handle->dest_file = strdup(file)))
        return NUC_AP_FATAL;

    if (!(handle->ar = archive_write_new())) {
        free(handle->dest_file);
        return NUC_AP_FATAL;
    }

    return NUC_AP_OK;
}


//// Unpacking Raw Data

EXPORT
void *nuc_arch_open_unpack(nuc_arch_read_callback read_fn, nuc_arch_skip_callback skip_fn, void *ctx, int *error) {
    nuc_arch_handle *handle = alloc_handle();
    int err;

    if (!handle) {
        *error = NUC_AP_FATAL;
        return NULL;
    }

    handle->mode = NUC_AP_MODE_UNPACK;

    handle->read_fn = read_fn;
    handle->skip_fn = skip_fn;
    handle->callback_ctx = ctx;

    if (!(handle->ar = archive_read_new())) {
        *error = NUC_AP_FATAL;
        goto free_mem;
    }

    if ((err = archive_read_support_filter_all(handle->ar)) != ARCHIVE_OK) {
        goto cleanup;
    }

    if ((err = archive_read_support_format_all(handle->ar)) != ARCHIVE_OK) {
        goto cleanup;
    }

    if ((err = archive_read_open(handle->ar, handle, open_callback, read_callback, close_callback)) != ARCHIVE_OK) {
        goto cleanup;
    }

    return handle;

cleanup:
    *error = err_code(handle, err);
    archive_read_free(handle->ar);

free_mem:

    free(handle);
    return NULL;
}

int open_callback(struct archive *ar, void *ctx) {
    return ARCHIVE_OK;
}

int close_callback(struct archive *ar, void *ctx) {
    return ARCHIVE_OK;
}

ssize_t read_callback(struct archive *ar, void *ctx, const void **buffer) {
    nuc_arch_handle *handle = ctx;

    ssize_t n = handle->read_fn(handle->callback_ctx, buffer);

    if (n < 0) {
	    /* TODO: Call archive_set_error */
	    return ARCHIVE_FATAL;
    }

    return n;
}

off_t skip_callback(struct archive *ar, void *ctx, off_t request) {
    nuc_arch_handle *handle = ctx;
    return handle->skip_fn(handle->callback_ctx, request);
}


//// Packing into Raw Data

EXPORT
void *nuc_arch_open_pack(nuc_arch_write_callback write_fn, void *ctx, int *error) {
    nuc_arch_handle *handle = alloc_handle();

    if (!handle) {
        *error = NUC_AP_FATAL;
        return NULL;
    }

    handle->mode = NUC_AP_MODE_PACK;
    handle->write_fn = write_fn;
    handle->callback_ctx = ctx;

    if (!(handle->ar = archive_write_new())) {
        *error = NUC_AP_FATAL;

        free(handle);
        return NULL;
    }

    return handle;
}

ssize_t unpack_write_callback(struct archive *ar, void *ctx, const void *buffer, size_t length) {
    nuc_arch_handle *handle = ctx;

    ssize_t n = handle->write_fn(handle->callback_ctx, buffer, length);

    if (n < 0) {
        /* TODO: Call archive_set_error */
        return -1;
    }

    return n;
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

    if (handle->dest_file) free(handle->dest_file);
    if (handle->ent) archive_entry_free(handle->ent);
    if (handle->tmp_file) fclose(handle->tmp_file);

    archive_write_free(handle->ar);

    return err;
}


//// Error Reporting

EXPORT
int nuc_arch_error_code(void *ctx) {
    nuc_arch_handle *handle = ctx;
    return archive_errno(handle->ar);
}

EXPORT
const char * nuc_arch_error_string(void *ctx) {
    nuc_arch_handle *handle = ctx;
    return archive_error_string(handle->ar);
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
int nuc_arch_next_entry(void *ctx, const char **path) {
    int err;
    nuc_arch_handle *handle = ctx;

    err = err_code(handle, archive_read_next_header(handle->ar, &handle->ent));

    if (err) return err;

    *path = archive_entry_pathname(handle->ent);

    return NUC_AP_OK;
}

EXPORT
const struct stat *nuc_arch_entry_stat(void *ctx) {
    nuc_arch_handle *handle = ctx;

    return archive_entry_stat(handle->ent);
}

EXPORT
const char *nuc_arch_entry_link_path(void *ctx) {
    nuc_arch_handle *handle = ctx;

    return archive_entry_hardlink(handle->ent);
}

EXPORT
const char *nuc_arch_entry_symlink_path(void *ctx) {
    nuc_arch_handle *handle = ctx;

    return archive_entry_symlink(handle->ent);
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

    if (dest->dest_file)
        return err_code(dest, archive_write_open_filename(dest->ar, dest->dest_file));
    else
        return err_code(dest, archive_write_open(dest->ar, dest, open_callback, unpack_write_callback, close_callback));
}


EXPORT
int nuc_arch_copy_last_entry_header(void *dest_handle, const void *src_handle) {
    const nuc_arch_handle *src = src_handle;
    nuc_arch_handle *dest = dest_handle;

    if (dest->ent)
        archive_entry_free(dest->ent);

    if (!(dest->ent = archive_entry_clone(src->ent)))
        return NUC_AP_FATAL;

    return NUC_AP_OK;
}

EXPORT
int nuc_arch_copy_last_entry_data(void *dest_handle, void *src_handle) {
    const nuc_arch_handle *src = src_handle;
    nuc_arch_handle *dest = dest_handle;

    int err;

    const void *buf;
    size_t size;
    off_t offset, last_offset = 0;

    while (!(err = err_code(src, archive_read_data_block(src->ar, &buf, &size, &offset)))) {
        if (nuc_arch_pack(dest, buf, size, offset - last_offset))
            return NUC_AP_FAILED;

        last_offset = offset + size;
    }

    if (err == NUC_AP_EOF)
        return nuc_arch_pack_finish(dest);

    return err == NUC_AP_OK || err == NUC_AP_EOF ? NUC_AP_OK : NUC_AP_FAILED;
}


EXPORT
int nuc_arch_create_entry(void *ctx, const char *path, const struct stat *st) {
    nuc_arch_handle *handle = ctx;

    create_entry(handle);

    if (!handle->ent) return NUC_AP_FATAL;

    archive_entry_set_pathname(handle->ent, path);
    archive_entry_copy_stat(handle->ent, st);

    return NUC_AP_OK;
}

void create_entry(nuc_arch_handle *handle) {
    if (handle->ent)
        archive_entry_free(handle->ent);

    handle->ent = archive_entry_new();
}

EXPORT
void nuc_arch_entry_set_path(void *ctx, const char *path) {
    nuc_arch_handle *handle = ctx;
    archive_entry_set_pathname(handle->ent, path);
}

EXPORT
void nuc_arch_entry_set_stat(void *ctx, const struct stat *st) {
    nuc_arch_handle *handle = ctx;
    archive_entry_copy_stat(handle->ent, st);
}

EXPORT
void nuc_arch_entry_set_link_path(void *ctx, const char *path) {
    nuc_arch_handle *handle = ctx;
    archive_entry_set_link(handle->ent, path);
}

EXPORT
void nuc_arch_entry_set_symlink_path(void *ctx, const char *path) {
    nuc_arch_handle *handle = ctx;
    archive_entry_set_symlink(handle->ent, path);
}

EXPORT
int nuc_arch_write_entry_header(void *ctx) {
    nuc_arch_handle *handle = ctx;

    // If size is unknown, create a temporary file to which the data
    // will be written. The data will be written to the actual archive
    // in nuc_arch_pack_finish.

    if (!archive_entry_size(handle->ent)) {
        char name[] = "/tmp/nucommander-tmpXXXXXX";
        int fd = mkstemp(name);

        if (fd >= 0) {
            handle->tmp_file = fdopen(fd, "wb+");
            unlink(name);

            return NUC_AP_OK;
        }
        else {
            return NUC_AP_FAILED;
        }
    }
    else {
        return err_code(handle, archive_write_header(handle->ar, handle->ent));
    }
}

/**
 * Writes a hole of size @a len to a file.
 *
 * @param f The file to write the hole to.
 * @param len The size of the hole.
 *
 * @return NUC_AP_OK if the hole was written successfully,
 *   NUC_AP_FAILED otherwise.
 */
static int write_hole_to_file(FILE *f, size_t len) {
    while (len--) {
        if (fputc(0, f) == EOF)
            return NUC_AP_FAILED;
    }

    return NUC_AP_OK;
}

/**
 * Writes a block of data to the temporary file storing the current
 * entry's data.
 *
 * @param handle Archive handle.
 *
 * @param buf Block of data to write.
 *
 * @param len Size of the block.
 *
 * @param offset Offset from the end of the last block at which to
 *   write the current block.
 *
 * @return NUC_AP_OK if the data was written successfully,
 *   NUC_AP_FAILED if there was an error.
 */
static int write_to_file(nuc_arch_handle *handle, const char *buf, size_t len, off_t offset) {
    if (offset) {
        if (write_hole_to_file(handle->tmp_file, offset))
            return NUC_AP_FAILED;
    }

    if (fwrite(buf, 1, len, handle->tmp_file) != len) {
        return NUC_AP_FAILED;
    }

    handle->ent_size += offset + len;
    return NUC_AP_OK;
}

EXPORT
int nuc_arch_pack(void *ctx, const char *buf, size_t len, off_t offset) {
    nuc_arch_handle *handle = ctx;

    // If a temporary file was created, write the data to it.
    if (handle->tmp_file)
        return write_to_file(handle, buf, len, offset);

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
    char *zeros = calloc(buf_size, 1);

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

EXPORT
int nuc_arch_pack_finish(void *ctx) {
    nuc_arch_handle *handle = ctx;

    if (handle->tmp_file) {
        FILE *f = handle->tmp_file;
        int err = NUC_AP_OK;

        archive_entry_set_size(handle->ent, handle->ent_size);

        handle->tmp_file = NULL;
        handle->ent_size = 0;

        if (err_code(handle, archive_write_header(handle->ar, handle->ent))) {
            err = NUC_AP_FAILED;
            goto cleanup;
        }

        if (fseek(f, 0L, SEEK_SET)) {
            err = NUC_AP_FAILED;
            goto cleanup;
        }

        char *buf = malloc(HOLE_BUF_SIZE);

        if (!buf) {
            err = NUC_AP_FATAL;
            goto cleanup;
        }

        ssize_t n;
        while ((n = fread(buf, 1, HOLE_BUF_SIZE, f))) {
            if (nuc_arch_pack(ctx, buf, n, 0)) {
                err = NUC_AP_FAILED;
                break;
            }
        }

        if (ferror(f))
            err = NUC_AP_FAILED;

        free(buf);

    cleanup:

        fclose(f);
        return err;
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
