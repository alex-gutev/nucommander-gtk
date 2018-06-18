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

#include "vfs.h"

#include "async_task.h"


using namespace nuc;


/// Object Creation

vfs::vfs() : queue(task_queue::create()) {
    monitor.signal_event().connect(sigc::mem_fun(*this, &vfs::file_event));
}

std::shared_ptr<vfs> vfs::create() {
    return std::make_shared<vfs>();
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

void vfs::finalize() {
    finalized = true;

    monitor.cancel();
    queue->cancel();
}


void vfs::read(const path_str& path) {
    cancel_update();

    add_read_task(path, false);
}

void vfs::add_read_task(const std::string &path, bool refresh) {
    using namespace std::placeholders;
    
    queue_task(std::bind(&vfs::read_path, _1, _2, path, refresh),
               std::bind(&vfs::finish_read, _1, _2, refresh));
}

void vfs::add_read_task(dir_type type, bool refresh) {
    using namespace std::placeholders;

    queue_task(std::bind(&vfs::list_dir, _1, _2, std::move(type), refresh),
               std::bind(&vfs::finish_read, _1, _2, refresh));
}

void vfs::add_refresh_task() {
    using namespace std::placeholders;

    add_read_task(cur_type, true);
}


/// Changing directory tree subdirectories

bool vfs::descend(const dir_entry &ent) {
    if (cur_tree->is_subdir(ent)) {
        add_read_subdir(ent.subpath());
        return true;
    }
    else {
        if (dir_type type = dir_type::get(cur_type.path(), ent)) {
            cancel_update();
            add_read_task(type, false);
            return true;
        }

        return false;     
    }
}

bool vfs::ascend() {
    if (!cur_tree->at_basedir()) {
        add_read_subdir(removed_last_component(cur_tree->subpath()));
        return true;
    }

    return false;
}

void vfs::add_read_subdir(const path_str &subpath) {
    using namespace std::placeholders;

    queue_task(std::bind(&vfs::read_subdir, _1, _2, subpath),
               std::bind(&vfs::finish_read_subdir, _1, _2, subpath));
}

void vfs::read_subdir(cancel_state &state, const path_str &subpath) {
    call_begin(state, false);

    if (const dir_tree::dir_map *dir = cur_tree->subpath_dir(subpath)) {
        cur_type.subpath(subpath);
        
        for (auto ent : *dir) {
            state.no_cancel([=] {
                call_new_entry(*ent.second, false);
            });
        }
        
        op_error = 0;
    }
    else {
        op_error = ENOENT;
    }
}

void vfs::finish_read_subdir(bool cancelled, const path_str &subpath) {
    queue->pause();
    
    queue_main([=] (vfs *self) {
        if (!cancelled && !self->op_error) {
            self->cur_tree->subpath(subpath);
        }

        self->cb_finish(cancelled, op_error, false);

        if (cancelled || self->op_error) {
            self->queue->resume();
        }
    });
}

/// Cancellation

void vfs::cancel_update() {
    // Prevent further update tasks from being queued
    monitor.pause();

    // Cancel any ongoing update tasks
    if (updating)
        queue->cancel();    
}

void vfs::cancel() {
    // If reading is true the directory monitor is paused. Thus if
    // reading is true there are no queued update tasks.
    
    if (reading) queue->cancel();
}


/// Committing reads

void vfs::commit_read() {
    if (new_tree) {
        cur_tree.swap(new_tree);
        new_tree = nullptr;
    }

    if (reading) {
        std::swap(cur_type, new_type);
        new_type = dir_type();
        
        monitor_dir();
    }
    else {
        resume_monitor();
    }

    queue->resume();
    
    reading = false;
    updating = false;
}


/// Read task

void vfs::read_path(cancel_state &state, const std::string &path, bool refresh) {
    list_dir(state, dir_type::get(path), refresh);
}

void vfs::list_dir(cancel_state& state, dir_type type, bool refresh) {
    std::unique_ptr<lister> listr(type.create_lister());
    
    state.no_cancel([=] {
        if (!refresh) reading = true;
        
        new_tree.reset(type.create_tree());
        new_type = std::move(type);
    });
    
    op_error = 0;
    call_begin(state, refresh);

    try {
        listr->open(new_type.path());
        
        lister::entry ent;
        struct stat st;

        while (listr->read_entry(ent)) {
            if (listr->entry_stat(st)) {
                add_entry(state, refresh, ent, st);
            }
        }
    }
    catch (lister::error &e) {
        op_error = e.code();
    }
}

void vfs::add_entry(cancel_state &state, bool refresh, const lister::entry &ent, const struct stat &st) {
    state.no_cancel([this, refresh, &ent, &st] {
        if (dir_entry *new_ent = new_tree->add_entry(ent, st)) {
            call_new_entry(*new_ent, refresh);
        }
    });
}


void vfs::finish_read(bool cancelled, bool refresh) {
    using namespace std::placeholders;

    if (cancelled || op_error) {
        new_tree = nullptr;
        new_type = dir_type();

        reading = false;

        if (!cancelled || !refresh) {
            if (refresh || !updating) {
                updating = false;
                
                // If refresh is true resumes directory monitor which was
                // created when this operation was queued. Otherwise
                // resumes the old directory monitor
                queue_main(std::bind(&vfs::resume_monitor, _1));
            }
            else {
                // Reread directory as update tasks were cancelled
                add_refresh_task();
                
                // Start new monitor for current directory.  Should be
                // paused as the update flag is modified as soon as an
                // event is received.
                queue_main(std::bind(&vfs::monitor_dir, _1, true));
            }
        }
    }

    call_finish(cancelled, !cancelled ? op_error : 0, refresh);
}

void vfs::resume_monitor() {
    monitor.resume();
}


//// Monitoring

void vfs::monitor_dir(bool paused) {
    if (!finalized) {
        monitor.monitor_dir(cur_type.path(), paused, cur_type.is_dir());
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
            queue_task(std::bind(&vfs::end_changes, _1, _2));
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
            sig_deleted.emit(cur_type.path());
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

void vfs::end_changes(cancel_state &state) {
    state.no_cancel([=] {
        cb_begin(true);

        for (auto &ent : *new_tree) {
            call_new_entry(ent.second, true);
        }

        call_finish(false, 0, true);
    });
}


void vfs::file_created(cancel_state &state, const path_str &path) {
    struct stat st;

    // Issue: Thee entry type is not obtained, instead the entry and
    // target file type are the same and obtained from the same stat
    // attributes.
    
    if (file_stat(path, &st)) {
        state.no_cancel([&] {
            const path_str name(file_name(path));
            
            remove_entry(name);
            new_tree->add_entry(dir_entry(name, st));
        });
    }
}

void vfs::file_changed(cancel_state &state, const path_str &path) {
    struct stat st;

    // Same entry type issue as file_created when the file is not
    // already in the tree.
    
    if (file_stat(path, &st)) {
        state.no_cancel([&] {
            const path_str name(file_name(path));
            
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

void vfs::file_deleted(cancel_state &state, const path_str &path) {
    state.no_cancel([=] {
        remove_entry(file_name(path));
    });
}

void vfs::file_renamed(cancel_state &state, const path_str &src, const path_str &dest) {
    bool exists = false;

    path_str src_name = file_name(src);
    path_str dest_name = file_name(dest);
    
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

void vfs::remove_entry(const path_str& name) {
    new_tree->index().erase(name);
}


bool vfs::file_stat(const path_str &path, struct stat *st) {
    return !stat(path.c_str(), st) || !lstat(path.c_str(), st);
}


//// Callbacks

void vfs::call_begin(cancel_state &state, bool refresh) {
    state.no_cancel([=] {
        cb_begin(refresh);
    });
}

void vfs::call_new_entry(dir_entry &ent, bool refresh) {
    cb_new_entry(ent, refresh);
}

void vfs::call_finish(bool cancelled, int error, bool refresh) {
    // Pause queue to prevent update tasks from being run before
    // commit_read is called.
    
    queue->pause();
    
    queue_main([=] (vfs *self) {
        self->cb_finish(cancelled, error, refresh);
        if (cancelled || error) queue->resume();

        if (refresh && !cancelled && !error) {
            // Check that subpath still exists
        }
    });
}

