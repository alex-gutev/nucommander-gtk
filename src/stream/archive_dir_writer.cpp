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

#include "archive_dir_writer.h"

#include "stream/archive_outstream.h"

#include "error_macros.h"

using namespace nuc;

archive_dir_writer::archive_dir_writer(paths::string arch_path, archive_plugin *plugin, paths::string subpath)
    : path(std::move(arch_path)), subpath(subpath), plugin(plugin) {
    int err;

    try_op([&] {
        if (!(in_handle = plugin->open(path.c_str(), NUC_AP_MODE_UNPACK, &err)))
            raise_error(errno, err);
    });

    try {
        open_temp();
    }
    catch (...) {
        close_handles();
        throw;
    }
}

void archive_dir_writer::open_temp() {
    // Create temporary file name

    tmp_path = path;
    tmp_path.resize(tmp_path.length() + 6, 'X');
    tmp_path.push_back(0);


    // Create temporary file

    try_op([=] {
        int outfd;
        if ((outfd = mkstemp(&tmp_path[0])) < 0)
            raise_error(errno);

        tmp_exists = true;

        ::close(outfd);
    });


    // Create output archive at the temporary file

    try_op([=] {
        int err;

        if (!(out_handle = plugin->open(tmp_path.c_str(), NUC_AP_MODE_PACK, &err)))
            raise_error(errno, err);
    });


    // Copy type of existing archive

    try_op([=] {
        nuc_arch_entry ent;
        if (int err = plugin->next_entry(in_handle, &ent) || plugin->copy_archive_type(out_handle, in_handle))
            raise_error(errno, err);
    });
}

archive_dir_writer::~archive_dir_writer() {
    close_handles();
}


void archive_dir_writer::close_handles() {
    if (in_handle) {
        plugin->close(in_handle);
        in_handle = nullptr;
    }

    if (out_handle) {
        plugin->close(out_handle);
        out_handle = nullptr;
    }

    if (tmp_exists)
        unlink(tmp_path.c_str());
}

void archive_dir_writer::close() {
    copy_old_entries();

    if (in_handle) {
        plugin->close(in_handle);
        in_handle = nullptr;
    }

    if (out_handle) {
        int err = plugin->close(out_handle);
        out_handle = nullptr;

        if (err)
            raise_error(errno, false);
    }

    // TODO: copy attributes of old archive file.

    TRY_OP(rename(tmp_path.c_str(), path.c_str()));

    tmp_exists = false;
}

void archive_dir_writer::copy_old_entries() {
    nuc_arch_entry ent;
    int err = 0;

    do {
        try_op([=] {
            if (plugin->copy_last_entry(out_handle, in_handle))
                raise_error(errno, err);
        });
    } while (next_entry(&ent));
}

bool archive_dir_writer::next_entry(nuc_arch_entry *ent) {
    int err;
    try_op([&] {
        err = plugin->next_entry(in_handle, ent);
        if (err < 0)
            raise_error(errno, err);
    });

    return err == NUC_AP_OK;
}

outstream * archive_dir_writer::create(const char *path, const struct stat *st, int flags) {
    nuc_arch_entry ent{};

    paths::string ent_path = paths::appended_component(subpath, path);

    ent.path = ent_path.c_str();
    ent.stat = st;

    create_entry(&ent);

    return new archive_outstream(plugin, out_handle);
}

void archive_dir_writer::symlink(const char *path, const char *target, const struct stat *st) {
    nuc_arch_entry ent{};

    paths::string ent_path = paths::appended_component(subpath, path);

    ent.path = ent_path.c_str();
    ent.symlink_dest = target;
    ent.stat = st;

    create_entry(&ent);
}

void archive_dir_writer::set_attributes(const char *path, const struct stat *st) {
    if (S_ISDIR(st->st_mode)) {
        nuc_arch_entry ent{};

        paths::string ent_path = subpath;
        paths::append_component(ent_path, path);

        ent.path = ent_path.c_str();
        ent.stat = st;

        create_entry(&ent);
    }
}

void archive_dir_writer::create_entry(nuc_arch_entry *ent) {
    try_op([&] {
        if (int err = plugin->create_entry(out_handle, ent))
            raise_error(errno, err);
    });
}
