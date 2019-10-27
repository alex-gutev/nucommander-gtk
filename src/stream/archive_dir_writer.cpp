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


//// Constructors

archive_dir_writer::archive_dir_writer(pathname arch_path, archive_plugin *plugin, pathname subpath)
    : path(std::move(arch_path)), plugin(plugin), subpath(subpath) {
    open_old();

    try {
        open_temp();
    }
    catch (...) {
        close_handles();
        throw;
    }
}

archive_dir_writer::archive_dir_writer(archive_plugin *plugin, const pathname &path, const pathname &subpath) : plugin(plugin), path(path), subpath(subpath) {}


//// Opening old archive and temporary new archive file

void archive_dir_writer::open_old() {
    try_op([this] {
        in_lister = std::unique_ptr<archive_lister>(new archive_lister(plugin, path));
    });
}

void archive_dir_writer::open_temp() {
    open_temp(path);
}

void archive_dir_writer::open_temp(const pathname &path) {
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


//// Cleanup

archive_dir_writer::~archive_dir_writer() {
    close_handles();
}


void archive_dir_writer::close_handles() {
    if (out_handle) {
        plugin->close(out_handle);
        out_handle = nullptr;
    }

    if (tmp_exists)
        unlink(tmp_path.c_str());
}

void archive_dir_writer::close() {
    open_old();
    copy_old_entries();

    if (out_handle) {
        int err = plugin->close(out_handle);
        out_handle = nullptr;

        if (err)
            raise_error(errno, false);
    }

    // TODO: copy attributes of old archive file.

    TRY_OP(::rename(tmp_path.c_str(), path.path().c_str()));

    tmp_exists = false;
}


//// Copying entries from old archive to new archive

void archive_dir_writer::get_old_entries() {
    lister::entry ent;
    bool copied = false;;

    while (next_entry(ent)) {
        pathname cpath = pathname(ent.name).canonicalize();

        add_old_entry(cpath, ent.type);

        add_parent_entries(cpath);

        if (!copied) {
            copy_archive_type();
            copied = true;
        }
    }

    in_lister.reset();
}

void archive_dir_writer::add_parent_entries(pathname path) {
    while(path = path.remove_last_component(), !path.empty()) {
        if (!add_old_entry(path, DT_DIR)) break;
    }
}

bool archive_dir_writer::add_old_entry(const pathname &path, int type) {
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
        if (int err = plugin->copy_archive_type(out_handle, in_lister->arch_handle()))
            // TODO: Obtain error description
            raise_error(errno, err);
    });
}

void archive_dir_writer::copy_old_entries() {
    lister::entry ent;
    int err = 0;

    while (next_entry(ent)) {
        auto it = old_entries.find(pathname(ent.name).canonicalize());

        if (it != old_entries.end()) {
            // FIXME: Get proper error code and error description

            TRY_OP_((err = plugin->copy_last_entry_header(out_handle, in_lister->arch_handle())),
                    raise_error(errno, err));

            // Set new path if the path has changed
            if (!it->second.new_path.empty()) {
                plugin->entry_set_path(out_handle, it->second.new_path.c_str());
            }

            TRY_OP_((err = plugin->write_entry_header(out_handle)),
                    raise_plugin_error(out_handle, err));

            TRY_OP_((err = plugin->copy_last_entry_data(out_handle, in_lister->arch_handle())),
                    raise_error(errno, err));
        }
    }
}

bool archive_dir_writer::next_entry(lister::entry &ent) {
    bool more = false;

    try_op([&] {
        more = in_lister->read_entry(ent);
    });

    return more;
}


//// Directory Operations

outstream * archive_dir_writer::create(const pathname &path, const struct stat *st, int flags) {
    // Default regular file stat structure
    struct stat reg{};
    reg.st_mode = S_IFREG | S_IRWXU;

    create_entry(flags & stream_flag_exclusive, path.path().c_str(), st ? st : &reg);

    return new archive_outstream(plugin, out_handle);
}

void archive_dir_writer::mkdir(const pathname &path, bool defer) {
    check_exists(subpath.append(path));

    if (!defer) {
        struct stat st{};

        st.st_mode = S_IFDIR;
        create_entry(false, path.path().c_str(), &st);
    }
}

void archive_dir_writer::symlink(const pathname &path, const pathname &target, const struct stat *st) {
    // Default symlink stat structure
    struct stat lnk{};
    lnk.st_mode = S_IFLNK;

    create_entry(true, path.path().c_str(), st ? st : &lnk, target.path().c_str());
}

void archive_dir_writer::set_attributes(const pathname &path, const struct stat *st) {
    if (st && S_ISDIR(st->st_mode)) {
        create_entry(true, path.path().c_str(), st);
    }
}

void archive_dir_writer::create_entry(bool check, const char *path, const struct stat *st, const char *symlink_dest) {
    pathname ent_path = subpath.append(path).canonicalize();

    if (check)
        check_exists(ent_path);
    else
        remove_old_entry(ent_path);

    int err;

    TRY_OP_((err = plugin->create_entry(out_handle, ent_path.c_str(), st)),
            raise_plugin_error(out_handle, err));

    if (symlink_dest)
        plugin->entry_set_symlink_path(out_handle, symlink_dest);

    TRY_OP_((err = plugin->write_entry_header(out_handle)),
            raise_plugin_error(out_handle, err));
}

void archive_dir_writer::check_exists(pathname path) {
    bool replace = false;

    path = path.canonicalize();

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

void archive_dir_writer::remove_old_entry(pathname path) {
    auto it = old_entries.find(path);

    if (it != old_entries.end()) {
        bool is_dir = it->second.type == DT_DIR;

        it = old_entries.erase(it);

        if (is_dir) {
            path = pathname(path, true);

            while (it != old_entries.end() && it->first.is_subpath(path)) {
                it = old_entries.erase(it);
            }
        }
    }
}


void archive_dir_writer::rename(const pathname &src, const pathname &dest) {
    check_exists(dest);

    pathname src_path = src.canonicalize();

    auto it = old_entries.find(src_path);

    if (it != old_entries.end()) {
        it->second.new_path = dest;

        if (it->second.type == DT_DIR) {
            size_t dir_len = src_path.path().length() + 1;

            ++it;
            for (auto end = old_entries.end(); it != end; ++it) {
                if (!it->first.is_subpath(src_path)) break;

                it->second.new_path = dest.append(it->first.path().substr(dir_len));
            }
        }
    }
}

void archive_dir_writer::remove(const pathname &path, bool relative) {
    pathname ent_path = (relative ? subpath.append(path) : path).canonicalize();

    // TODO: Raise an error if there is no such entry
    remove_old_entry(ent_path);
}
