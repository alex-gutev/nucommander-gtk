/*
 * NuCommander
 * Copyright (C) 2018-2019  Alexander Gutev <alex.gutev@gmail.com>
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

#include "vfs.h"

#include "tasks/async_task.h"
#include "operations/copy.h"

using namespace nuc;


/// Object Creation

vfs::vfs() : queue(task_queue::create()) {
    monitor.signal_event().connect(sigc::mem_fun(*this, &vfs::file_event));
}

std::shared_ptr<vfs> vfs::create() {
    struct enable_make_shared : public vfs {
        using vfs::vfs;
    };

    return std::make_shared<enable_make_shared>();
}


/// Accessors

vfs::deleted_signal vfs::signal_deleted() {
    return sig_deleted;
}


/// Private Template Implementations

template <typename F>
void vfs::queue_task(F task) {
    std::shared_ptr<vfs> ptr = shared_from_this();

    queue->add([=] (cancel_state &state) {
        task(ptr.get(), state);
    });
}

template <typename T, typename F>
void vfs::queue_task(T task, F finish) {
    std::shared_ptr<vfs> ptr = shared_from_this();

    queue->add([=] (cancel_state &state) {
        task(ptr.get(), state);
    }, [=] (bool cancelled) {
        finish(ptr.get(), cancelled);
    });
}

template <typename F>
void vfs::queue_main(F fn) {
    auto ptr = std::weak_ptr<vfs>(shared_from_this());

    dispatch_main([=] {
        if (auto this_ptr = ptr.lock()) {
            fn(this_ptr.get());
        }
    });
}

template <typename F>
void vfs::queue_main_wait(F fn) {
    queue->pause();
    queue_main(fn);
}


void vfs::finalize() {
    finalized = true;

    monitor.cancel();
    queue->cancel();
}


void vfs::read(const paths::pathname& path, std::shared_ptr<delegate> del) {
    cancel_update();

    add_read_task(path, false, del);
}

void vfs::add_read_task(const paths::pathname &path, bool refresh, std::shared_ptr<delegate> del) {
    using namespace std::placeholders;

    auto tstate = std::make_shared<read_dir_state>(refresh, del);

    queue_task(std::bind(&vfs::read_path, _1, _2, tstate, path),
               std::bind(&vfs::finish_read, _1, _2, tstate));
}

void vfs::add_read_task(std::shared_ptr<dir_type> type, bool refresh,  std::shared_ptr<delegate> del) {
    using namespace std::placeholders;

    auto tstate = std::make_shared<read_dir_state>(refresh, del);
    tstate->type = type;

    queue_task(std::bind(&vfs::list_dir, _1, _2, tstate),
               std::bind(&vfs::finish_read, _1, _2, tstate));
}

void vfs::add_refresh_task() {
    using namespace std::placeholders;

    if (auto del = cb_changed())
        add_read_task(dtype->copy(), true, del);
}


/// Changing directory tree subdirectories

bool vfs::descend(const dir_entry &ent, std::shared_ptr<delegate> del) {
    if (cur_tree->is_subdir(ent)) {
        add_read_subdir(ent.subpath(), del);
        return true;
    }
    else {
        if (auto type = dir_type::get(dtype, ent)) {
            cancel_update();
            add_read_task(type, false, del);
            return true;
        }

        return false;
    }
}

bool vfs::ascend(std::shared_ptr<delegate> del) {
    if (!cur_tree->at_basedir()) {
        add_read_subdir(cur_tree->subpath().remove_last_component(), del);
        return true;
    }

    return false;
}

void vfs::add_read_subdir(const paths::pathname &subpath, std::shared_ptr<delegate> del) {
    using namespace std::placeholders;

    monitor.pause();

    auto tstate = std::make_shared<read_subdir_state>(subpath, del);

    queue_task(std::bind(&vfs::read_subdir, _1, _2, tstate),
               std::bind(&vfs::finish_read_subdir, _1, _2, tstate));
}

void vfs::read_subdir(cancel_state &state, std::shared_ptr<read_subdir_state> tstate) {
    call_begin(state, tstate->m_delegate);

    state.no_cancel([=] {
        if (const dir_tree::dir_map *dir = cur_tree->subpath_dir(tstate->subpath)) {
            for (auto ent : *dir) {
                tstate->m_delegate->new_entry(*ent.second);
            }
        }
        else {
            tstate->error = ENOENT;
        }
    });
}

void vfs::finish_read_subdir(bool cancelled, std::shared_ptr<read_subdir_state> tstate) {
    queue_main_wait([=] (vfs *self) {
        int error = tstate->error;

        if (!cancelled && !error) {
            self->dtype->subpath(tstate->subpath);
            self->cur_tree->subpath(tstate->subpath);
        }

        tstate->m_delegate->finish(cancelled, error);

        self->resume_monitor();
        self->queue->resume();
    });
}

void vfs::refresh_subdir() {
    if (!cur_tree->at_basedir() && !cur_tree->subpath_dir(cur_tree->subpath())) {
        paths::pathname subpath = cur_tree->subpath();

        cur_tree->subpath(subpath.remove_last_component());

        // While the subpath is not at the base directory and the
        // subdirectory does not exist
        while (!cur_tree->at_basedir() && !cur_tree->subpath_dir(cur_tree->subpath())) {
            cur_tree->subpath(cur_tree->subpath().remove_last_component());
            subpath = subpath.remove_last_component();
        }

        cur_tree->subpath(subpath);
        sig_deleted.emit(dtype->path().append(subpath));
    }
}


/// Cancellation

void vfs::cancel_update() {
    // Prevent further update tasks from being queued
    monitor.pause();

    // Cancel any ongoing update tasks
    if (updating)
        queue->cancel();
}

bool vfs::cancel() {
    // If reading is true the directory monitor is paused. Thus if
    // reading is true there are no queued update tasks.

    if (reading) {
        queue->cancel();
        return true;
    }

    return false;
}


/// Read task

void vfs::read_path(cancel_state &state, std::shared_ptr<read_dir_state> tstate, const paths::pathname &path) {
    tstate->type = dir_type::get(path);
    list_dir(state, tstate);
}

void vfs::list_dir(cancel_state& state, std::shared_ptr<read_dir_state> tstate) {
    state.no_cancel([&] {
        if (!tstate->refresh) reading = true;
    });

    tstate->tree.reset(tstate->type->create_tree());

    call_begin(state, tstate->m_delegate);

    try {
        std::unique_ptr<lister> listr(tstate->type->create_lister());

        lister::entry ent;
        struct stat st;

        while (listr->read_entry(ent)) {
            if (listr->entry_stat(st)) {
                add_entry(state, *tstate, ent, st);
            }
        }
    }
    catch (const nuc::error &e) {
        tstate->error = e.code();
    }
}

void vfs::add_entry(cancel_state &state, read_dir_state &tstate, const lister::entry &ent, const struct stat &st) {
    state.no_cancel([&tstate, &ent, &st] {
        if (dir_entry *new_ent = tstate.tree->add_entry(ent, st)) {
            tstate.m_delegate->new_entry(*new_ent);
        }
    });
}

void vfs::finish_read(bool cancelled, std::shared_ptr<read_dir_state> tstate) {
    queue_main_wait([=] (vfs *self) {
        int error = tstate->error;

        // Swap new tree and old tree and set new directory type
        if (!cancelled && !error) {
            self->cur_tree.swap(tstate->tree);
            self->dtype = tstate->type;
        }

        // Call finish callback
        tstate->m_delegate->finish(cancelled, error);

        // Start new monitor or reset old monitor
        self->start_new_monitor(cancelled, error, tstate->refresh);

        // Clear new_tree, new_type and flags
        self->clear_flags();

        // Resume task queue
        self->queue->resume();

        if (tstate->refresh && !cancelled && !error) {
            // Check that the current tree's subpath still exists.
            self->refresh_subdir();
        }
    });
}


void vfs::clear_flags() {
    reading = false;
    updating = false;
}

void vfs::start_new_monitor(bool cancelled, int error, bool refresh) {
    if (!cancelled && !error) {
        if (!refresh)
            monitor_dir();
        else
            resume_monitor();
    }
    else if (updating) { // Some updates were not applied
        // Reread directory
        add_refresh_task();

        // Start new monitor for current directory. Should be
        // paused as the update flag is modified as soon as an
        // event is received.

        monitor_dir(true);
    }
    else { // No non-applied updates
        // Resume old monitor
        resume_monitor();
    }
}

void vfs::resume_monitor() {
    monitor.resume();
}


//// Monitoring

void vfs::monitor_dir(bool paused) {
    if (!finalized) {
        monitor.monitor_dir(dtype->path(), paused, dtype->is_dir());
    }
}

void vfs::file_event(dir_monitor::event e) {
    using namespace std::placeholders;

    switch (e.type()) {
        // Event stages
        case dir_monitor::EVENTS_BEGIN:
            updating = true;
            queue_task(std::bind(&vfs::begin_changes, _1, _2));
            break;

        case dir_monitor::EVENTS_END:
            if (auto del = cb_changed())
                queue_task(std::bind(&vfs::end_changes, _1, _2, del));
            break;

        // File events
        case dir_monitor::FILE_CREATED:
            queue_task(std::bind(&vfs::file_created, _1, _2, e.src()));
            break;

        case dir_monitor::FILE_DELETED:
            queue_task(std::bind(&vfs::file_deleted, _1, _2, e.src()));
            break;

        case dir_monitor::FILE_MODIFIED:
            queue_task(std::bind(&vfs::file_changed, _1, _2, e.src()));
            break;

        case dir_monitor::FILE_RENAMED:
            queue_task(std::bind(&vfs::file_renamed, _1, _2, e.src(), e.dest()));
            break;

        // Directory events
        case dir_monitor::DIR_DELETED:
            monitor.cancel();

            // Reset to base directory so that attempts to ascend up
            // the tree fail.
            cur_tree->subpath("");
            sig_deleted.emit(dtype->path());
            break;

        case dir_monitor::DIR_MODIFIED:
            add_refresh_task();
            break;
    }
}


void vfs::begin_changes(cancel_state &state) {
    state.no_cancel([=] {
        // Create new directory tree directly since this event is only
        // called for regular directories
        new_tree.reset(new dir_tree());

        // Copy current file index to new tree's file index
        new_tree->index() = cur_tree->index();
    });
}

void vfs::end_changes(cancel_state &state, std::shared_ptr<delegate> del) {
    state.no_cancel([&] {
        del->begin();

        for (auto &ent : *new_tree) {
            del->new_entry(ent.second);
        }

        finish_updates(del);
    });
}

void vfs::finish_updates(std::shared_ptr<delegate> del) {
    queue_main_wait([=] (vfs *self) {
        self->cur_tree.swap(new_tree);

        del->finish(false, 0);

        self->clear_flags();
        self->queue->resume();
    });
}


void vfs::file_created(cancel_state &state, const paths::string &path) {
    struct stat st;

    // Issue: The entry type is not obtained, instead the entry and
    // target file type are the same and obtained from the same stat
    // attributes.

    if (file_stat(path, &st)) {
        state.no_cancel([&] {
            const paths::string name(paths::pathname(path).basename());

            remove_entry(name);
            new_tree->add_entry(dir_entry(name, st));
        });
    }
}

void vfs::file_changed(cancel_state &state, const paths::string &path) {
    struct stat st;

    // Same entry type issue as file_created when the file is not
    // already in the tree.

    if (file_stat(path, &st)) {
        state.no_cancel([&] {
            const paths::string name(paths::pathname(path).basename());

            dir_entry *ent = new_tree->get_entry(name);

            if (ent) {
                ent->attr(st);
            }
            else {
                new_tree->add_entry(dir_entry(name, st));
            }
        });
    }
}

void vfs::file_deleted(cancel_state &state, const paths::string &path) {
    state.no_cancel([=] {
        remove_entry(paths::pathname(path).basename());
    });
}

void vfs::file_renamed(cancel_state &state, const paths::string &src, const paths::string &dest) {
    bool exists = false;

    paths::string src_name = paths::pathname(src).basename();
    paths::string dest_name = paths::pathname(dest).basename();

    state.no_cancel([&] {
        dir_entry *ent = new_tree->get_entry(src_name);

        if (ent) {
            exists = true;

            ent->orig_subpath(dest_name);

            remove_entry(dest_name);
            new_tree->add_entry(std::move(*ent));

            remove_entry(src_name);
        }
    });

    if (!exists) {
        file_created(state, dest);
    }
}

void vfs::remove_entry(const paths::string& name) {
    new_tree->index().erase(name);
}


bool vfs::file_stat(const paths::string &path, struct stat *st) {
    return !stat(path.c_str(), st) || !lstat(path.c_str(), st);
}


//// Callbacks

void vfs::call_begin(cancel_state &state, std::shared_ptr<delegate> del) {
    state.no_cancel([=] {
        del->begin();
    });
}


//// Accessing Files on Disk

task_queue::task_type vfs::access_file(const dir_entry &ent, std::function<void(const paths::pathname &)> fn) {
    using namespace std::placeholders;

    if (!dtype->is_dir()) {
        return make_unpack_task(dtype, ent.orig_subpath(), [=] (const char *path) {
            fn(paths::pathname(path));
        });
    }
    else {
        paths::pathname full_path = dtype->path().append(ent.orig_subpath());

        return std::bind(fn, full_path);
    }
}
