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

#include "dir_lister.h"

#include "async_task.h"


using namespace nuc;

/// Object Creation

vfs::vfs() : queue(task_queue::create()) {
    monitor.signal_event().connect(sigc::mem_fun(*this, &vfs::file_event));
}

std::shared_ptr<vfs> vfs::create() {
    return std::make_shared<vfs>();
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

/*
 * Assumptions:
 *
 *    read() should only be called on the main thread, and not more than
 *    once until the finish callback is called.
 *
 *    file events are only delivered on the main thread.
 *
 *    commit_read() is only called on the main thread.
 *
 * read() pauses the directory monitor and cancels any ongoing update
 * tasks. Since the file events are delivered on the same thread that
 * read() is called on, no update tasks will be queued after read() is
 * called.
 *
 * If queue.cancel() is called after the background task completes, it
 * will only result in an additional expense (canceling an empty task
 * queue) and not a race condition.
 */


void vfs::read(const path_str& path) {
    cancel_update();

    add_read_task(path, false);
}

void vfs::add_read_task(const std::string &path, bool refresh) {
    using namespace std::placeholders;
    
    queue_task(std::bind(&vfs::read_dir, _1, _2, path, refresh),
               std::bind(&vfs::finish_read, _1, _2, path, refresh));
}

void vfs::cancel_update() {
    // Prevent further update tasks from being queued
    monitor.pause();

    // Cancel any ongoing update tasks
    if (updating)
        queue->cancel();    
}

/*
 * Since the directory monitor is paused until commit_read() is called
 * (which clears the 'reading' flag), if 'reading' is true then there
 * are no queued update tasks.
 */
void vfs::cancel() {
    if (reading) queue->cancel();
}

/*
 * commit_read() clears the old directory tree, and begins monitoring
 * the new directory (in the case of a read operation), resumes the
 * task queue (in the case of an update operation).
 *
 * The purpose of this function is for the controller to signal that
 * it will no longer be using the old directory list. Until this
 * function is called it has to be kept in memory.
 *
 * 'reading' and 'updating' may both be true only if an update task
 * was cancelled before beginning the read task, since read() pauses
 * the directory monitor, preventing further update tasks from being
 * queued.
 */

void vfs::commit_read() {
    cur_tree.swap(new_tree);
    new_tree.clear();

    if (reading) {
        monitor_dir(cur_path);
    }
    else {
        resume_monitor();
    }

    queue->resume();
    
    reading = false;
    updating = false;
}


/// Read tasks

void vfs::read_dir(cancel_state &state, const std::string &path, bool refresh) {
    std::unique_ptr<lister> listr(new dir_lister());

    state.no_cancel([=] {
        if (!refresh) reading = true;
        
        new_tree.clear();
    });
    
    op_status = 0;
    call_begin(state, refresh);
    
    try {
        listr->open(path);
        
        lister::entry ent;
        struct stat st;

        while (listr->read_entry(ent)) {
            if (listr->entry_stat(st)) {
                add_entry(state, ent, st);
            }
        }
    }
    catch (lister::error &e) {
        op_status = e.code();
    }
}

void vfs::add_entry(cancel_state &state, const lister::entry &ent, const struct stat &st) {
    state.no_cancel([this, &ent, &st] {
        dir_entry &new_ent = new_tree.add_entry(ent, st);
        call_new_entry(new_ent);
    });
}


void vfs::finish_read(bool cancelled, const path_str &path, bool refresh) {
    using namespace std::placeholders;
    
    if (cancelled || op_status) {
        new_tree.clear();

        reading = false;

        if (refresh) {
            updating = false;

            // Resume monitor which was created when this operation was initiated
            queue_main(std::bind(&vfs::resume_monitor, _1));
        }
        else if (updating) {
            // Read directory as update tasks were cancelled
            add_read_task(cur_path, true);
            
            // Start new monitor for current directory.  Should be
            // paused as the update flag is modified as soon as an
            // event is received.
            queue_main(std::bind(&vfs::monitor_dir, _1, cur_path, true));
        }
        else {
            // Resume old directory monitor
            queue_main(std::bind(&vfs::resume_monitor, _1));
        }
    }
    else {
        cur_path = path;
    }

    call_finish(cancelled, !cancelled ? op_status : 0, refresh);
}

void vfs::resume_monitor() {
    monitor.resume();
}


//// Monitoring

void vfs::monitor_dir(const path_str &path, bool paused) {
    if (!finalized) {
        monitor.monitor_dir(path, paused);
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
            break;
            
        case dir_monitor::DIR_MODIFIED:
            break;
            
        case dir_monitor::DIR_RENAMED:
            break;
    }
}


/* 
 * When begin_changes() is called, the 'updating' flag is set to true,
 * thus if a read task is to be queued, the queued update task will
 * first be cancelled.
 *
 * Since begin_changes() (called in response to a file event) and
 * read() are both only called on the main thread, there is no
 * possibility of a data race. 
 *
 * If read() runs first, the directory monitor is paused and
 * begin_changes() is not called.
 *
 * If begin_changes() runs first, the 'updating' flag is set and the
 * update task is queued. When read() runs the queued update task is
 * cancelled.
 */

void vfs::begin_changes(cancel_state &state) {
    state.no_cancel([=] {
        new_tree.clear();
        new_tree.parse_dirs(false);

        // Copy current file index to new tree's file index
        new_tree.index() = cur_tree.index();
    });
}

/*
 * end_changes() pauses the task queue in order to prevent read tasks
 * being run before commit_read() is called to commit the
 * update. 
 *
 * Since the queue is paused from inside the running task (in the "no
 * cancel" state), no other task can be run until this task returns,
 * and thus even if read(...) is executed before queue.pause(), the
 * read task will not be run until the queue is resumed, when
 * commit_read() is called.
 */

void vfs::end_changes(cancel_state &state) {
    state.no_cancel([=] {
        cb_begin(true);

        for (auto &ent : new_tree) {
            call_new_entry(ent.second);
        }

        call_finish(false, 0, true);
    });
}


void vfs::file_created(cancel_state &state, const path_str &path) {
    struct stat st;

    if (file_stat(path, &st)) {
        state.no_cancel([&] {
            new_tree.add_entry(dir_entry(file_name(path), st));
        });
    }    
}

void vfs::file_changed(cancel_state &state, const path_str &path) {
    struct stat st;

    if (file_stat(path, &st)) {
        state.no_cancel([&] {
            dir_entry *ent = new_tree.get_entry(file_name(path));

            if (ent) {
                ent->attr(st);
            }
            else {
                new_tree.add_entry(dir_entry(file_name(path), st));
            }
        });
    }    
}

void vfs::file_deleted(cancel_state &state, const path_str &path) {
    state.no_cancel([=] {
        new_tree.index().erase(file_name(path));
    });
}

void vfs::file_renamed(cancel_state &state, const path_str &src, const path_str &dest) {
    state.no_cancel([&] {
        dir_entry *ent = new_tree.get_entry(file_name(src));

        if (ent) {
            path_str dest_file = file_name(dest);
            
            new_tree.index().erase(dest_file);
            
            new_tree.add_entry(dir_entry(dest_file, ent->attr()));
            new_tree.index().erase(ent->file_name());
        }
    });
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

void vfs::call_new_entry(dir_entry &ent) {
    cb_new_entry(ent);
}

void vfs::call_finish(bool cancelled, int error, bool refresh) {
    // Pause queue to prevent read tasks from being run before
    // commit_read is called.
    
    queue->pause();
    
    queue_main([=] (vfs *self) {
        self->cb_finish(cancelled, error, refresh);
        if (cancelled || error) queue->resume();
    });
}

