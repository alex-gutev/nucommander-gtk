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
    open_old();

    try {
        open_temp();
    }
    catch (...) {
        close_handles();
        throw;
    }
}

void archive_dir_writer::open_old() {
    try_op([this] {
        int err;

        if (!(in_handle = plugin->open(path.c_str(), NUC_AP_MODE_UNPACK, &err)))
            raise_error(errno, err);
    });
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


    get_old_entries();
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

    TRY_OP(::rename(tmp_path.c_str(), path.c_str()));

    tmp_exists = false;
}


void archive_dir_writer::get_old_entries() {
    nuc_arch_entry ent;
    bool copied = false;;

    while (next_entry(&ent)) {
        paths::string cpath = paths::canonicalized_path(ent.path);

        add_old_entry(cpath, IFTODT(ent.stat->st_mode));

        add_parent_entries(cpath);

        if (!copied) {
            copy_archive_type();
            copied = true;
        }
    }

    plugin->close(in_handle);
    in_handle = nullptr;
}

void archive_dir_writer::add_parent_entries(paths::string path) {
    while(paths::remove_last_component(path), path.length()) {
        if (!add_old_entry(path, DT_DIR)) break;
    }
}

bool archive_dir_writer::add_old_entry(const paths::string &path, int type) {
    auto pair = old_entries.insert(std::make_pair(path, type));

    if (!pair.second) {
        if (type == DT_DIR && pair.first->second.type != DT_DIR) {
            pair.first->second = DT_DIR;
            return true;
        }

        return false;
    }

    return true;
}

void archive_dir_writer::copy_archive_type() {
    try_op([=] {
        if (int err = plugin->copy_archive_type(out_handle, in_handle))
            // TODO: Obtain error description
            raise_error(errno, err);
    });
}

void archive_dir_writer::copy_old_entries() {
    nuc_arch_entry ent;
    int err = 0;

    open_old();

    while (next_entry(&ent)) {
        auto it = old_entries.find(paths::canonicalized_path(ent.path));

        if (it != old_entries.end()) {
            // Clear all fields of the nuc_arch_entry struct
            ent = nuc_arch_entry();

            // If the entry should be recreated at a new path, set it
            // in the nuc_arch_entry struct.
            if (!it->second.new_path.empty()) {
                ent.path = it->second.new_path.c_str();
            }

            try_op([=] {
                if (plugin->copy_last_entry(out_handle, in_handle, &ent))
                    // TODO: Obtain error description
                    raise_error(errno, err);
            });
        }
    }
}

bool archive_dir_writer::next_entry(nuc_arch_entry *ent) {
    int err;
    try_op([&] {
        err = plugin->next_entry(in_handle, ent);
        if (err < 0)
            raise_plugin_error(in_handle, err);
    });

    return err == NUC_AP_OK;
}

outstream * archive_dir_writer::create(const paths::string &path, const struct stat *st, int flags) {
    create_entry(path.c_str(), st);

    return new archive_outstream(plugin, out_handle);
}

void archive_dir_writer::mkdir(const paths::string &path) {
    // Check whether an entry exists in the archive with the
    // same name, however don't actually create a directory
    // entry as it is implicitly created when its child
    // entries. The actual directory entry is only created
    // when the its attributes are set.

    check_exists(path.c_str());
}

void archive_dir_writer::symlink(const paths::string &path, const paths::string &target, const struct stat *st) {
    create_entry(path.c_str(), st, target.c_str());
}

void archive_dir_writer::set_attributes(const paths::string &path, const struct stat *st) {
    if (S_ISDIR(st->st_mode)) {
        create_entry(path.c_str(), st);
    }
}

void archive_dir_writer::create_entry(const char *path, const struct stat *st, const char *symlink_dest) {
    nuc_arch_entry ent{};

    paths::string ent_path = paths::canonicalized_path(paths::appended_component(subpath, path));

    ent.path = ent_path.c_str();
    ent.stat = st;
    ent.symlink_dest = symlink_dest;

    create_entry(&ent);
}

void archive_dir_writer::create_entry(nuc_arch_entry *ent) {
    check_exists(ent->path);

    try_op([&] {
        if (int err = plugin->create_entry(out_handle, ent))
            raise_plugin_error(out_handle, err);
    });
}

void archive_dir_writer::check_exists(const char *path) {
    bool replace = false;

    global_restart overwrite(restart("overwrite", [&replace, this, path] (const nuc::error &e, boost::any) {
        replace = true;
        remove_old_entry(path);
    },
    [] (const nuc::error &e) {
        return e.code() == EEXIST;
    }));

    global_restart duplicate(restart("duplicate", [&replace] (const nuc::error &e, boost::any) {
        replace = true;
    },
    [] (const nuc::error &e) {
        return e.code() == EEXIST;
    }));

    try_op([&] {
        if (!replace && old_entries.count(path))
            throw file_error(EEXIST, error::type_create_file, true, path);
    });
}

void archive_dir_writer::remove_old_entry(paths::string path) {
    auto it = old_entries.find(path);

    if (it != old_entries.end()) {
        bool is_dir = it->second.type == DT_DIR;

        it = old_entries.erase(it);

        if (is_dir) {
            paths::append_component(path, "");

            while (it != old_entries.end() && paths::is_prefix(path, it->first)) {
                it = old_entries.erase(it);
            }
        }
    }
}


    // TODO: Implement renaming.
void archive_dir_writer::rename(const paths::string &src, const paths::string &dest) {
}

void archive_dir_writer::remove(const paths::string &path, bool relative) {
    paths::string ent_path = paths::canonicalized_path(relative ? paths::appended_component(subpath, path) : path);

    // TODO: Raise an error if there is no such entry
    remove_old_entry(ent_path);
}
