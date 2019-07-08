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

///////////////////////////////////////////////////////////////////////////////
//                           Background Task State                           //
///////////////////////////////////////////////////////////////////////////////

/**
 * Background Task State
 *
 * Stores the task, onto which background tasks are queued and a
 * pointer to the VFS object.
 */
struct vfs::background_task_state : std::enable_shared_from_this<vfs::background_task_state> {
    /**
     * Pointer to the VFS object.
     *
     * Should only be accessed and modified from the main thread.
     */
    class vfs *vfs;

    /**
     * Flag: true if a read operation is ongoing.
     */
    std::atomic<bool> reading{false};
    /**
     * Flag: true if updates to the directory tree have been
     * received which have not been applied yet.
     */
    std::atomic<bool> updating{false};


    /**
     * Background task queue.
     */
    std::shared_ptr<task_queue> queue;

    /**
     * Constructor.
     *
     * @param vfs Pointer to the VFS object.
     */
    background_task_state(class vfs *vfs);

    /**
     * Queue the function fn to be run on the main thread.
     *
     * The function fn is called with the pointer to the VFS object
     * passed as an argument. If the VFS object is no longer in memory
     * the function is not invoked.
     */
    template <typename F>
    void queue_main(F fn);

    /**
     * Same as queue_main however the task_queue is paused prior
     * to dispatching @a fn on the main thread.
     *
     * @param      fn Function to execute on the main thread.
     */
    template <typename F>
    void queue_main_wait(F fn);
};

/**
 * Background Task.
 */
struct vfs::background_task {
    /** Background Task State */
    std::weak_ptr<vfs::background_task_state> vfs_tasks;

    background_task(std::weak_ptr<background_task_state> tasks)
        : vfs_tasks(tasks) {}

    /**
     * Queues the function @a fn to be executed on the main thread, by
     * vfs_tasks->queue_main, if vfs_tasks is still in memory.
     *
     * @param fn The function to execute on the main thread.
     */
    template<typename F>
    void queue_main(F fn);

    /**
     * Queues the function @a fn to be executed on the main thread, by
     * vfs_tasks->queue_main_wait, if vfs_tasks is still in memory.
     *
     * @param fn The function to execute on the main thread.
     */
    template<typename F>
    void queue_main_wait(F fn);
};

///////////////////////////////////////////////////////////////////////////////
//                                 Read Task                                 //
///////////////////////////////////////////////////////////////////////////////

/**
 * Read Directory Task State.
 */
struct vfs::read_dir_task : public vfs::background_task, std::enable_shared_from_this<vfs::read_dir_task> {
    /** Flag: Is this a refresh operation */
    bool refresh;

    /** Operation Delegate */
    std::shared_ptr<vfs::delegate> m_delegate;

    /** Error Code */
    std::atomic<int> error{0};

    /** dir_type of the directory to read */
    std::shared_ptr<dir_type> type;
    /** dir_tree into which directory is read */
    std::shared_ptr<dir_tree> tree;

    /**
     * Constructor.
     *
     * @param refresh True if this is a refresh operation.
     * @param tasks Background Task State.
     * @param del Operation Delegate.
     */
    read_dir_task(bool refresh, std::weak_ptr<vfs::background_task_state> tasks, std::shared_ptr<vfs::delegate> del)
        : background_task(tasks), refresh(refresh), m_delegate(del) {}

    /* Disable Copying */
    read_dir_task(const read_dir_task &) = delete;
    read_dir_task &operator=(const read_dir_task &) = delete;

    /**
     * Reads the directory at @a path.
     *
     * @param state The cancellation state.
     * @param path  The path to read.
     */
    void read_path(cancel_state &state, const pathname &path);

    /**
     * Reads the directory, by creating the lister object using
     * `type`.
     *
     * @param state The cancellation state.
     */
    void list_dir(cancel_state &state);

    /**
     * Read task finish callback function.
     *
     * @param cancelled True if the task was cancelled.
     */
    void finish_read(bool cancelled);


    /**
     * Adds an entry to directory tree 'tstate->tree'. The adding
     * of the entry is performed with the cancellation state set
     * to "no cancel".
     *
     * @param state Cancellation state.
     * @param ent The entry to add.
     * @param st State attributes of the entry to add.
     */
    void add_entry(cancel_state &state, const lister::entry &ent, const struct stat &st);
};


///////////////////////////////////////////////////////////////////////////////
//                              Read Subdir Task                             //
///////////////////////////////////////////////////////////////////////////////

/**
 * Read Subdirectory Task State.
 */
struct vfs::read_subdir_task : public vfs::background_task, std::enable_shared_from_this<vfs::read_subdir_task> {
    /** Subpath to the subdirectory */
    pathname subpath;

    /**
     * Directory containing the virtual directory.
     */
    std::shared_ptr<dir_tree> tree;

    /** Operation Delegate */
    std::shared_ptr<vfs::delegate> m_delegate;

    /** Error code */
    std::atomic<int> error{0};

    /**
     * Constructor.
     *
     * @param path Subdirectory subpath.
     * @param del The delegate object for the read operation.
     */
    read_subdir_task(pathname path, std::weak_ptr<dir_tree> tree, std::shared_ptr<vfs::background_task_state> tasks, std::shared_ptr<delegate> del)
        : background_task(tasks), subpath(std::move(path)), tree(tree), m_delegate(del) {}

    /* Disable copying */
    read_subdir_task(const read_subdir_task &) = delete;
    read_subdir_task &operator=(const read_subdir_task &) = delete;

    /**
     * Read subdirectory task.
     *
     * Reads the subdirectory of the directory tree if it exists. The
     * subdirectory is obtained and the callback functions are called
     * as though the directory is being read from disk.
     *
     * @param state  The cancellation state.
     */
    void read_subdir(cancel_state &state);

    /**
     * Read subdirectory task finish callback.
     *
     * Queues a task on the main thread, which sets the subpath of
     * the directory tree and calls the finish callback.
     *
     * @param cancelled True if the task was cancelled.
     */
    void finish_read_subdir(bool cancelled);
};


///////////////////////////////////////////////////////////////////////////////
//                            Update Directory Task                          //
///////////////////////////////////////////////////////////////////////////////

/**
 * Update Task State
 */
struct vfs::update_task : public vfs::background_task {
    /** Operation Delegate */
    std::shared_ptr<delegate> m_delegate;

    /** Directory Tree which is updated */
    std::shared_ptr<dir_tree> tree;

    /**
     * Constructor.
     *
     * @param tasks Background Task State.
     * @param del Operation Delegate.
     * @param tree Directory tree to update.
     */
    update_task(std::weak_ptr<background_task_state> tasks, std::shared_ptr<delegate> del, std::shared_ptr<dir_tree> tree)
        : background_task(tasks), m_delegate(del), tree(tree) {}
};


///////////////////////////////////////////////////////////////////////////////
//                                 Utilities                                 //
///////////////////////////////////////////////////////////////////////////////

/**
 * Calls the begin method of an operation delegate. The method is
 * called with the cancellation state in the "no cancel" state.
 *
 * @param state Cancellation State.
 * @param delegate Operation Delegate
 */
static void call_begin(cancel_state &state, std::shared_ptr<vfs::delegate> delegate);


///////////////////////////////////////////////////////////////////////////////
//                             VFS Implementation                            //
///////////////////////////////////////////////////////////////////////////////

// Constructors ///////////////////////////////////////////////////////////////

vfs::vfs() : tasks(std::make_shared<background_task_state>(this)) {
    monitor.signal_event().connect(sigc::mem_fun(*this, &vfs::file_event));
}

vfs::~vfs() {
    tasks->vfs = nullptr;
}

vfs::background_task_state::background_task_state(class vfs *vfs)
    : vfs(vfs), queue(task_queue::create()) {}


// Accessors //////////////////////////////////////////////////////////////////

vfs::deleted_signal vfs::signal_deleted() {
    return sig_deleted;
}


// Queuing Background Tasks //////////////////////////////////////////////////

template <typename F>
void vfs::background_task_state::queue_main(F fn) {
    auto ptr = std::weak_ptr<background_task_state>(shared_from_this());

    dispatch_main([=] {
        if (auto this_ptr = ptr.lock()) {
            if (this_ptr->vfs)
                fn(this_ptr->vfs);
        }
    });
}

template <typename F>
void vfs::background_task_state::queue_main_wait(F fn) {
    queue->pause();
    queue_main(fn);
}


template<typename F>
void vfs::background_task::queue_main(F fn) {
    if (auto tasks = vfs_tasks.lock())
        tasks->queue_main(fn);
}

template<typename F>
void vfs::background_task::queue_main_wait(F fn) {
    if (auto tasks = vfs_tasks.lock())
        tasks->queue_main_wait(fn);
}


// Initiating Background Read Tasks ///////////////////////////////////////////

void vfs::read(const pathname& path, std::shared_ptr<delegate> del) {
    cancel_update();

    add_read_task(path, false, del);
}

void vfs::add_read_task(const pathname &path, bool refresh, std::shared_ptr<delegate> del) {
    auto task = std::make_shared<read_dir_task>(refresh, tasks, del);

    tasks->queue->add([=] (cancel_state &state) {
        task->read_path(state, path);
    }, [=] (bool cancelled) {
        task->finish_read(cancelled);
    });
}

void vfs::add_read_task(std::shared_ptr<dir_type> type, bool refresh,  std::shared_ptr<delegate> del) {
    auto task = std::make_shared<read_dir_task>(refresh, tasks, del);
    task->type = type;

    tasks->queue->add([=] (cancel_state &state) {
        task->list_dir(state);
    }, [=] (bool cancelled) {
        task->finish_read(cancelled);
    });
}

void vfs::add_refresh_task() {
    if (auto del = cb_changed())
        add_read_task(dtype, true, del);
}


// Read Task //////////////////////////////////////////////////////////////////

void vfs::read_dir_task::read_path(nuc::cancel_state &state, const nuc::pathname &path) {
    type = dir_type::get(path);
    list_dir(state);
}

void vfs::read_dir_task::list_dir(nuc::cancel_state &state) {
    if (!refresh) {
        state.no_cancel([&] {
            if (auto tasks = vfs_tasks.lock())
                if (!refresh) tasks->reading = true;
        });
    }

    tree.reset(type->create_tree());

    call_begin(state, m_delegate);

    try {
        std::unique_ptr<lister> listr(type->create_lister());

        lister::entry ent;
        struct stat st;

        while (listr->read_entry(ent)) {
            if (listr->entry_stat(st)) {
                add_entry(state, ent, st);
            }
        }
    }
    catch (const nuc::error &e) {
        error = e.code();
    }
}

void vfs::read_dir_task::add_entry(nuc::cancel_state &state, const lister::entry &ent, const struct stat &st) {
    state.no_cancel([this, &ent, &st] {
        if (dir_entry *new_ent = tree->add_entry(ent, st)) {
            m_delegate->new_entry(*new_ent);
        }
    });
}

void vfs::read_dir_task::finish_read(bool cancelled) {
    auto state = shared_from_this();

    queue_main_wait([state, cancelled] (vfs *self) {
        // Swap new tree and old tree and set new directory type
        if (!cancelled && !state->error) {
            self->cur_tree.swap(state->tree);
            self->dtype = state->type;
        }

        // Call finish callback
        state->m_delegate->finish(cancelled, state->error);

        // Start new monitor or reset old monitor
        self->start_new_monitor(cancelled, state->error, state->refresh);

        // Clear new_tree, new_type and flags
        self->clear_flags();

        // Resume task queue
        self->tasks->queue->resume();

        if (state->refresh && !cancelled && !state->error) {
            // Check that the current tree's subpath still exists.
            self->refresh_subdir();
        }
    });
}


void call_begin(cancel_state &state, std::shared_ptr<vfs::delegate> delegate) {
    state.no_cancel([=] {
        delegate->begin();
    });
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

void vfs::add_read_subdir(const pathname &subpath, std::shared_ptr<delegate> del) {
    monitor.pause();

    auto task = std::make_shared<read_subdir_task>(subpath, cur_tree, tasks, del);

    tasks->queue->add([=] (cancel_state &state) {
        task->read_subdir(state);
    }, [=] (bool cancelled) {
        task->finish_read_subdir(cancelled);
    });
}


// Read Subdirectory Task /////////////////////////////////////////////////////

void vfs::read_subdir_task::read_subdir(cancel_state &state) {
    call_begin(state, m_delegate);

    state.no_cancel([=] {
        if (const dir_tree::dir_map *dir = tree->subpath_dir(subpath)) {
            for (auto ent : *dir) {
                m_delegate->new_entry(*ent.second);
            }
        }
        else {
            error = ENOENT;
        }
    });
}

void vfs::read_subdir_task::finish_read_subdir(bool cancelled) {
    auto state = shared_from_this();

    queue_main_wait([state, cancelled] (vfs *self) {
        if (!cancelled && !state->error) {
            self->dtype = self->dtype->change_subpath(state->subpath);
            self->cur_tree->subpath(state->subpath);
        }

        state->m_delegate->finish(cancelled, state->error);

        self->resume_monitor();
        self->tasks->queue->resume();
    });
}

void vfs::refresh_subdir() {
    if (!cur_tree->at_basedir() && !cur_tree->subpath_dir(cur_tree->subpath())) {
        pathname subpath = cur_tree->subpath();

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
    if (tasks->updating)
        tasks->queue->cancel();
}

bool vfs::cancel() {
    // If reading is true the directory monitor is paused. Thus if
    // reading is true there are no queued update tasks.

    if (tasks->reading) {
        tasks->queue->cancel();
        return true;
    }

    return false;
}


void vfs::clear_flags() {
    tasks->reading = false;
    tasks->updating = false;
}

void vfs::start_new_monitor(bool cancelled, int error, bool refresh) {
    if (!cancelled && !error) {
        if (!refresh)
            monitor_dir();
        else
            resume_monitor();
    }
    else if (tasks->updating) { // Some updates were not applied
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
    monitor.monitor_dir(dtype->path(), paused, dtype->is_dir());
}

/**
 * Event handler for the EVENTS_BEGIN event. This event is
 * sent before the first event in a series of events.
 *
 * @param state Cancellation State.
 *
 * @param new_tree Directory tree to which updates should be applied.
 *
 * @param cur_tree Old directory tree.
 */
static void begin_changes(cancel_state &state, std::shared_ptr<dir_tree> new_tree, std::shared_ptr<dir_tree> cur_tree) {
    state.no_cancel([=] {
        // Copy current file index to new tree's file index
        new_tree->index() = cur_tree->index();
    });
}

void vfs::end_changes(cancel_state &state, std::shared_ptr<update_task> tstate) {
    state.no_cancel([&] {
        tstate->m_delegate->begin();

        for (auto &ent : tstate->tree->index()) {
            tstate->m_delegate->new_entry(ent.second);
        }

        finish_updates(tstate);
    });
}

void vfs::finish_updates(std::shared_ptr<update_task> state) {
    state->queue_main_wait([=] (vfs *self) {
        self->cur_tree.swap(state->tree);

        state->m_delegate->finish(false, 0);

        self->clear_flags();
        self->tasks->queue->resume();
    });
}


/**
 * Obtains the stat attributes of a file. First the stat
 * system call is attempted, if that fails the lstat system
 * call is attempted.
 *
 * @param path The file path.
 *
 * @param st   Pointer to the stat struct where the stat
 *             attributes will be read into
 *
 * @return true if successful, false if both stat and lstat
 *         failed.
 */
static bool file_stat(const pathname::string &path, struct stat *st) {
    return !stat(path.c_str(), st) || !lstat(path.c_str(), st);
}

/**
 * Removes an entry from the new directory tree (new_tree).
 *
 * @param subpath The subpath of the entry
 * @param tree The tree from which to remove the entry.
 */
static void remove_entry(const pathname::string& name, std::shared_ptr<dir_tree> tree) {
    tree->index().erase(name);
}


/**
 * Handler functions for the create, change, delete and rename
 * file events.
 */
static void file_created(cancel_state &state, std::shared_ptr<dir_tree> tree, const pathname::string &path) {
    struct stat st;

    // Issue: The entry type is not obtained, instead the entry and
    // target file type are the same and obtained from the same stat
    // attributes.

    if (file_stat(path, &st)) {
        state.no_cancel([&] {
            const pathname::string name(pathname(path).basename());

            remove_entry(name, tree);
            tree->add_entry(dir_entry(name, st));
        });
    }
}

static void file_changed(cancel_state &state, std::shared_ptr<dir_tree> tree, const pathname::string &path) {
    struct stat st;

    // Same entry type issue as file_created when the file is not
    // already in the tree.

    if (file_stat(path, &st)) {
        state.no_cancel([&] {
            const pathname::string name(pathname(path).basename());

            dir_entry *ent = tree->get_entry(name);

            if (ent) {
                ent->attr(st);
            }
            else {
                tree->add_entry(dir_entry(name, st));
            }
        });
    }
}

static void file_deleted(cancel_state &state, std::shared_ptr<dir_tree> tree, const pathname::string &path) {
    state.no_cancel([=] {
        remove_entry(pathname(path).basename(), tree);
    });
}

static void file_renamed(cancel_state &state, std::shared_ptr<dir_tree> tree, const pathname::string &src, const pathname::string &dest) {
    bool exists = false;

    pathname::string src_name = pathname(src).basename();
    pathname::string dest_name = pathname(dest).basename();

    state.no_cancel([&] {
        dir_entry *ent = tree->get_entry(src_name);

        if (ent) {
            exists = true;

            ent->orig_subpath(dest_name);

            remove_entry(dest_name, tree);
            tree->add_entry(std::move(*ent));

            remove_entry(src_name, tree);
        }
    });

    if (!exists) {
        file_created(state, tree, dest);
    }
}


void vfs::file_event(dir_monitor::event e) {
    using namespace std::placeholders;

    switch (e.type()) {
        // Event stages
        case dir_monitor::EVENTS_BEGIN:
            tasks->updating = true;
            new_tree = std::make_shared<dir_tree>();
            tasks->queue->add(std::bind(&begin_changes, _1, new_tree, cur_tree));
            break;

        case dir_monitor::EVENTS_END:
            if (auto del = cb_changed())
                tasks->queue->add(std::bind(&vfs::end_changes, _1, std::make_shared<update_task>(tasks, del, new_tree)));
            break;

        // File events
        case dir_monitor::FILE_CREATED:
            tasks->queue->add(std::bind(&file_created, _1, new_tree, e.src()));
            break;

        case dir_monitor::FILE_DELETED:
            tasks->queue->add(std::bind(&file_deleted, _1, new_tree, e.src()));
            break;

        case dir_monitor::FILE_MODIFIED:
            tasks->queue->add(std::bind(&file_changed, _1, new_tree, e.src()));
            break;

        case dir_monitor::FILE_RENAMED:
            tasks->queue->add(std::bind(&file_renamed, _1, new_tree, e.src(), e.dest()));
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


//// Accessing Files on Disk

task_queue::task_type vfs::access_file(const dir_entry &ent, std::function<void(const pathname &)> fn) {
    using namespace std::placeholders;

    if (!dtype->is_dir()) {
        return make_unpack_task(dtype, ent.orig_subpath(), [=] (const char *path) {
            fn(pathname(path));
        });
    }
    else {
        pathname full_path = dtype->path().append(ent.orig_subpath());

        return std::bind(fn, full_path);
    }
}
