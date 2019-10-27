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

#ifndef NUC_PLUGINS_ARCHIVE_PLUGIN_API_H
#define NUC_PLUGINS_ARCHIVE_PLUGIN_API_H

#include "archive_plugin_types.h"

/**
 * Contains the prototypes of the archive plugin API functions.
 */


//// Obtaining a handle

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


//// Error Reporting API

/**
 * Returns a unique code identifying the last error that occurred.
 *
 * @param handle Handle to the archive.
 *
 * @return Error code.
 */
int nuc_arch_error_code(void *handle);
/**
 * Returns a string describing the last error that occurred.
 *
 * @param handle Handle to the archive.
 *
 * @return The error string.
 */
const char * nuc_arch_error_string(void *handle);


//// Unpacking API

/**
 * Creates an unpacker for an archive where the actual archive data is
 * returned by a read callback function rather than read directly from
 * a file.
 *
 * The purpose of this function is to allow unpacking of archives
 * directly from memory. This is used to read archives nested within
 * other archives.
 *
 * @param read_fn Read callback function.
 *
 * @param skip_fn Skip callback function. May be NULL in which case
 *   the plugin should call manually skip over data returned by the
 *   read callback.
 *
 * @param ctx Context pointer which is passed as the first argument to
 *   the read and skip callback functions.
 *
 * @param error Pointer to a integer which is set to an error constant
 *   if an error occurs.
 *
 * @return Handle to the archive unpacker, or NULL if an error
 *   occurred, in which case an appropriate NUC_AP_ error code should
 *   be stored in the location pointed to by @a error.
 */
void *nuc_arch_open_unpack(nuc_arch_read_callback read_fn, nuc_arch_skip_callback skip_fn, void *ctx, int *error);


/**
 * Retrieves the next entry from the archive.
 *
 * @param handle The handle to the archive.
 *
 * @param path Pointer to a cstring pointer which is set to the name
 *   of the entry.
 *
 * @return NUC_AP_OK (0) if successful, NUC_AP_EOF if there
 *    are no more entries otherwise, a NUC_AP_ error constant
 *    value if there was an error. A detailed error code is
 *    stored in errno.
 */
int nuc_arch_next_entry(void *handle, const char ** path);

/**
 * Returns the stat attributes of the current entry, read using
 * nuch_arch_next_entry.
 *
 * @param handle The handle to the archive.
 *
 * @return Pointer to a stat struct.
 */
const struct stat *nuc_arch_entry_stat(void *handle);

/**
 * Returns the path to the linked file, if the current entry is a hard
 * link.
 *
 * @param handle The handle to the archive.
 *
 * @return Path to the linked file or null if the entry is not a hard
 *   link.
 */
const char *nuc_arch_entry_link_path(void *handle);
/**
 * Returns the path to the target of the symbolic link, if the current
 * entry is a symbolic link.
 *
 * @param handle The handle to the archive.
 *
 * @return The target of the link or null if the entry is not a
 *   symbolic link.
 */
const char *nuc_arch_entry_symlink_path(void *handle);


/**
 * Reads a block of data from the last entry read. Data can only be
 * read from regular files.
 *
 * @param handle Handle to the archive.
 *
 * @param buf Pointer to a pointer to a char where the pointer to the
 *    first byte of the block will be stored.
 *
 * @param len Pointer to a size_t where the size of the block will be
 *    stored.
 *
 * @param offset Pointer to an off_t where the offset of the start of
 *    the current block (in bytes) will be stored.
 *
 * @return NUC_AP_OK (0) if the block was read successfully,
 *    NUC_AP_EOF if the end of the entry was reached. a NUC_AP_ error
 *    constant value is returned if there was an error, with a more
 *    detailed error code stored in errno.
 */
int nuc_arch_unpack(void *handle, const char **buf, size_t *len, off_t *offset);

/**
 * Sets a progress callback which is called when unpacking individual
 * files.
 *
 * @param handle   The handle to the archive.
 * @param callback Pointer to the callback function.
 * @param ctx      Context pointer passed to the callback.
 */
void nuc_arch_set_callback(void *handle, nuc_arch_progress_fn callback, void *ctx);


//// Packing API

/**
 * Creates a packer for an archive where the actual archive data is
 * written by calling the write callback function @a write_fn.
 *
 * @param write_fn Write callback function.
 *
 * @param ctx Context pointer which should be passed as the first
 *   argument to the write callback.
 *
 * @param error Pointer to a integer which is set to an error constant
 *   if an error occurs.
 *
 * @return Handle to the archive unpacker, or NULL if an error
 *   occurred, in which case an appropriate NUC_AP_ error code should
 *   be stored in the location pointed to by @a error.
 */
void *nuc_arch_open_pack(nuc_arch_write_callback write_fn, void *ctx, int *error);

/**
 * Copies the archive type of the open, for unpacking, archive @a
 * src_handle to the destination archive, open for packing, @a
 * dest_handle.
 *
 * In order for the type of the source archive to be known reliably,
 * at least a single entry must have been read.
 *
 * @param dest_handle The handle of the destination archive, of which
 *    to set the type.
 *
 * @param src_handle The handle of the source archive, the type of
 *    which will be set as the type of the destination handle.
 *
 * @return NUC_AP_OK (0) if the archive's type was set successfully. A
 *    NUC_AP_ error constant value is returned if there was an error,
 *    with a more detailed error code stored in errno.
 */
int nuc_arch_copy_archive_type(void *dest_handle, const void *src_handle);

/**
 * Copies the header, i.e. the metadata, of the last entry read from
 * the archive with handle @a src_handle, into the archive handle @a
 * dest_handle.
 *
 * The entry header is not actually written to the archive, thus may
 * be changed using nuc_arch_entry_set_X
 * functions. nuc_arch_write_entry_header should be called to actually
 * write the header to the archive.
 *
 * @param dest_handle Handle of the destination archive into which to
 *   copy the entry header.
 *
 * @param src_handle Handle of the source archive from which to copy
 *   the header.
 *
 * @return NUC_AP_OK (0) if the header was copied successfully. A
 *    NUC_AP_ error constant value is returned if there was an error,
 *    with a more detailed error code stored in errno.
 */
int nuc_arch_copy_last_entry_header(void *dest_handle, const void *src_handle);
/**
 * Copies the data of the last entry, read from the archive with
 * handle @a src_handle, into the archive with handle @a dest_handle.
 *
 * The handle @a dest_handle must be in a state in which entry data
 * can be written using nuc_arch_pack.
 *
 * @param dest_handle Handle of the destination archive into which to
 *   write the data.
 *
 * @param src_handle Handle of the source archive from which to read
 *   the data.
 *
 * @return NUC_AP_OK (0) if the data was copied successfully. A
 *    NUC_AP_ error constant value is returned if there was an error,
 *    with a more detailed error code stored in errno.
 */
int nuc_arch_copy_last_entry_data(void *dest_handle, void *src_handle);


/**
 * Creates a new entry, the attributes of which can be set using the
 * following functions:
 *
 *  - nuc_arch_entry_set_path
 *  - nuc_arch_entry_set_stat
 *  - nuc_arch_entry_set_link_path
 *  - nuc_arch_entry_set_symlink_path
 *
 * Once the attributes have been set nuc_arch_write_entry_header must
 * be called to actually create the entry in the archive. After the
 * entry is created its data is written using nuc_arch_pack.
 *
 * @param handle Handle of the archive into which to create the entry.
 *
 * @param path The entry's path, may be changed later using
 *   nuc_arch_entry_set_path.
 *
 * @param st The entry's stat atttributes, may be changed later using
 *   nuc_arch_entry_set_stat.
 *
 * @return NUC_AP_OK (0) if the entry was created successfully. A
 *    NUC_AP_ error constant value is returned if there was an error,
 *    with a more detailed error code stored in errno.
 */
int nuc_arch_create_entry(void *handle, const char *path, const struct stat *st);
/**
 * Sets the path of the entry, created by nuc_arch_create_entry.
 *
 * This function must be called after nuc_arch_create_entry is called,
 * and returns NUC_AP_OK, and before nuc_arch_write_entry_header is
 * called.
 *
 * @param handle Handle to the archive.
 * @param path The path to set.
 */
void nuc_arch_entry_set_path(void *handle, const char *path);
/**
 * Sets the stat attributes of the entry, created by
 * nuc_arch_create_entry.
 *
 * This function must be called after nuc_arch_create_entry is called,
 * and returns NUC_AP_OK, and before nuc_arch_write_entry_header is
 * called.
 *
 * @param handle Handle to the archive.
 * @param st Stat attributes to set.
 */
void nuc_arch_entry_set_stat(void *handle, const struct stat *st);
/**
 * Sets the path to the linked file of the entry, created by
 * nuc_arch_create_entry. This effectively converts the entry to a
 * hard link.
 *
 * This function must be called after nuc_arch_create_entry is called,
 * and returns NUC_AP_OK, and before nuc_arch_write_entry_header is
 * called.
 *
 * @param handle Handle to the archive.
 * @param path Path to the linked file.
 */
void nuc_arch_entry_set_link_path(void *handle, const char *path);
/**
 * Sets the symbolic link target of the entry, created by
 * nuc_arch_create_entry. This may only be called if the entry is a
 * symbolic link.
 *
 * This function must be called after nuc_arch_create_entry is called,
 * and returns NUC_AP_OK, and before nuc_arch_write_entry_header is
 * called.
 *
 * @param handle Handle to the archive.
 */
void nuc_arch_entry_set_symlink_path(void *handle, const char *path);

/**
 * Writes the header, i.e. the metadata, of the entry, created by the
 * last call to nuc_arch_create_entry, to the archive.
 *
 * The entry's attributes may not be changed after this function is
 * called.
 *
 * Only after this function returns NUC_AP_OK may the entry's data be
 * written (if any) using nuc_arch_pack.
 *
 * @param handle Handle to the archive.
 *
 * @return NUC_AP_OK (0) if the header was written successfully. A
 *    NUC_AP_ error constant value is returned if there was an error,
 *    with a more detailed error code stored in errno.
 */
int nuc_arch_write_entry_header(void *handle);

/**
 * Writes data to the last entry created in the archive. This function
 * may only be used if the last entry created was a regular file.
 *
 * @param handle Handle of the archive.
 *
 * @param buf Pointer to the block of data to write.
 *
 * @param len Size (number of bytes) of the block.
 *
 * @param offset Number of zero bytes to write between the last byte
 *    written and the first byte of the block.
 *
 * @return NUC_AP_OK (0) if the data was written successfully. A
 *    NUC_AP_ error constant value is returned if there was an error,
 *    with a more detailed error code stored in errno.
 */
int nuc_arch_pack(void *handle, const char *buf, size_t len, off_t offset);

/**
 * Finishes writing data to the entry. This signals to the plugin that
 * no more data will be written, and that all data, passed to the
 * plugin, in the preceding pack calls should be written to the
 * archive file.
 *
 * @param handle Handle of the archive.
 *
 * @return NUC_AP_OK (0) if this was written successful. A NUC_AP_
 *    error constant value is returned if there was an error, with a
 *    more detailed error code stored in errno.
 */
int nuc_arch_pack_finish(void *handle);

#endif

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
