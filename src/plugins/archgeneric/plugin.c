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
#include <errno.h>

#include <archive.h>
#include <archive_entry.h>

#include "plugins/archive_plugin_api.h"

#define EXPORT __attribute__((visibility("default")))

typedef struct nuc_arch_handle {
	struct archive *ar;
	struct archive_entry *ent;

	nuc_arch_progress_fn callback;
	void *ctx;
} nuc_arch_handle;

static nuc_arch_prog_act call_callback(nuc_arch_handle *handle, nuc_arch_prog_type type, int error, size_t bytes);

static int err_code(int err);

EXPORT
void *nuc_arch_open(const char *file, int mode, int *error) {
    nuc_arch_handle *handle = malloc(sizeof(nuc_arch_handle));
    
    if (!handle) {
        return NULL;
    }
    
    int err = NUC_AP_OK;
    
    handle->callback = NULL;
    
    if (!(handle->ar = archive_read_new())) {
        err = NUC_AP_FATAL;
        goto free_mem;
    }
    
    if ((err = archive_read_support_filter_all(handle->ar)) != ARCHIVE_OK) {
        errno = archive_errno(handle->ar);
        goto cleanup;
    }
    
    if ((err = archive_read_support_format_all(handle->ar)) != ARCHIVE_OK) {
        errno = archive_errno(handle->ar);
        goto cleanup;
    }
    
    if ((err = archive_read_open_filename(handle->ar, file, 10240)) != ARCHIVE_OK) {
        errno = archive_errno(handle->ar);
        goto cleanup;
    }
    
    return handle;
    
cleanup:
    *error = err_code(err);
    archive_read_free(handle->ar);
    
free_mem:
    free(handle);
    return NULL;
}

EXPORT
int nuc_arch_close(void *ctx) {
    nuc_arch_handle *handle = ctx;
    
    archive_read_close(handle->ar);
    archive_read_free(handle->ar);
    free(handle);
    
    return NUC_AP_OK;
}

EXPORT
void nuc_arch_set_callback(void *plg_ctx, nuc_arch_progress_fn callback, void *cb_ctx) {
    nuc_arch_handle *handle = plg_ctx;
    
    handle->callback = callback;
    handle->ctx = cb_ctx;
}

EXPORT
int nuc_arch_next_entry(void *ctx, nuc_arch_entry *ent) {
    int err;
    nuc_arch_handle *handle = ctx;
    
    err = archive_read_next_header(handle->ar, &handle->ent);
    
    if (err != ARCHIVE_OK) {
        return err_code(err);
    }
    
    ent->path = archive_entry_pathname(handle->ent);
    
    ent->link_dest = archive_entry_hardlink(handle->ent);
    ent->symlink_dest = archive_entry_symlink(handle->ent);
    
    ent->stat = archive_entry_stat(handle->ent);
    
    // TODO: Check for errors, as those functions can technically fail
    
    return NUC_AP_OK;
}

EXPORT
int nuc_arch_unpack(void *ctx, const char **buf, size_t *size, off_t *offset) {
	nuc_arch_handle *handle = ctx;

	return err_code(archive_read_data_block(handle->ar, (const void **)buf, size, offset));
}


nuc_arch_prog_act call_callback(nuc_arch_handle *handle, nuc_arch_prog_type type, int error, size_t bytes) {
    if (handle->callback) {
        return handle->callback(handle->ctx, type, error, bytes);
    }
    
    return error ? NUC_AP_ACT_ABORT : NUC_AP_ACT_CONTINUE;
}


static int err_code(int err) {
    // Set errno in case of error
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
