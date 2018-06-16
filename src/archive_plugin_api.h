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

#ifndef NUC_ARCHIVE_PLUGIN_API_H
#define NUC_ARCHIVE_PLUGIN_API_H

#include "archive_plugin_types.h"

/**
 * Contains the prototypes of the archive plugin API functions.
 */

/**
 * Opens the archive and returns a "handle" to the archive
 * which can be passed to the remaining functions.
 *
 * @param path Path to the archive file
 *
 * @param mode Open mode, one of the nuc_arch_open_mode
 *    constants. NUC_AP_MODE_UNPACK to open the archive for
 *    reading the archive's contents, NUC_AP_MODE_PACK to
 *    create a new archive to which files will be added.
 *
 * @param error Pointer to an integer which will store the
 *    error code if any.
 *
 * @return A handle to the archive if the archive was opened
 *    successfully, NULL if the archive could not be opened,
 *    the error code is stored in the location pointed to by
 *    @a error.
 */
void *nuc_arch_open(const char *file, int mode, int *error);

/**
 * Closes the archive and frees all resource.
 *
 * In case there was an error closing the archive, the
 * resources held by the plugin should still be freed.
 *
 * @param handle The archive handle.
 *
 * @return 0 if successful, otherwise a non-zero value is
 *   returned and the error is stored in errno.
 */
int nuc_arch_close(void *handle);


/**
 * Unpacking API
 */

/**
 * Reads the next entry's metadata from the archive.
 *
 * @param handle The handle to the archive.
 *
 * @param entry  Pointer to a nuc_arch_entry struct where the
 *    entry metadata is read into.
 *
 * @return NUC_AP_OK (0) if successful, NUC_AP_EOF if there
 *    are no more entries otherwise, a NUC_AP_ error constant
 *    value if there was an error. A detailed error code is
 *    stored in errno.
 */
int nuc_arch_read_entry(void *handle, nuc_arch_entry *ent);

/**
 * Unpacks the last entry read. Only regular file entries can be
 * unpacked.
 *
 * @param handle The handle to the archive.
 *
 * @param destfd The file descriptor into which to write the file's
 *               contents.
 */
int nuc_arch_unpack_entry(void *handle, int destfd);

/**
 * Sets a progress callback which is called when unpacking individual
 * files.
 *
 * @param handle   The handle to the archive.
 * @param callback Pointer to the callback function.
 * @param ctx      Context pointer passed to the callback.
 */
void nuc_arch_set_callback(void *handle, nuc_arch_progress_fn callback, void *ctx);

#endif

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
