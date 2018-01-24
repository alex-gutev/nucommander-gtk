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

#ifndef NUC_LISTER_H
#define NUC_LISTER_H

#include <sys/stat.h>
#include <dirent.h>

#include <errno.h>

#include <atomic>

#include "types.h"

namespace nuc {
    /**
     * Abstract filelister interface.
     *
     * This class provides a generic interface for listing the contents of
     * directories, regardless of whether the directories are OS
     * directories or virtual directories such as archives.
     *
     * The interface also provides an implementation of a thread-safe
     * callback mechanism.
     */
    class lister {
        /**
         * The reference count, initially 2 as the object holds a
         * reference to itself while the background operation is ongoing
         * and the external controller object holds a reference to this
         * object.
         *
         * Can only be decremented (using the release method) not
         * incremented, as that would require a fully thread safe
         * reference counting mechanism.
         */
        std::atomic<int> ref_count {2};
        
        
        /**
         * The cancellation state, which is one of:
         *
         * CAN_CANCEL   - It is safe to cancel the operation at this point.
         * IN_CALLBACK  - Currently in the middle of the callback.
         * CANCELLED    - The operation has been cancelled.
         */
        std::atomic<int> cancel_state;
        /**
          * True if the operation has finished.
          */
        std::atomic_flag finished;
        
    public:
        /** 
         * Operation stage constants, passed in the stage parameter of the
         * callback.
         */
        enum stage_t {
            /** 
             * The operation is about to begin, after the callback
             * returns.
             */
            BEGIN = 0,
            /**
             * An entry has been read, the 'ent' and 'st' parameters will
             * point to the entry and stat structures.
             */
            ENTRY,
            /**
             * The operation has finished, the callback will no longer be
             * called.
             */
            FINISH,
        };
        
        /**
         * A simple entry structures containing two fields:
         *
         * name: The name of the entry as a c string.
         * type: The type of the entry as a dirent constant.
         */
        struct entry {
            const char *name;
            uint8_t type;
        };
        
        /**
         * Callback function pointer type, takes the following arguments
         * in order:
         *
         * fl:     Pointer to the filelister object.
         * stage:  The current stage of the operation (as a stage_t constant).
         * ent:    The entry struct of the current entry (NULL if stage != ENTRY).
         * st:     The stat attributes of the current entry (NULL if stage != ENTRY).
         */
        typedef void(*callback_fn)(lister *l, stage_t stage, const entry *ent, const struct stat *st);
        
        
        /**
         * Creator function pointer type, takes the following arguments in
         * order:
         *
         * cb:    The callback function pointer.
         * ctx:   Value of the ctx member variable.
         * type:  An extra initialization argument for use by the filelister object.
         *
         * Returns a pointer to the filelister object.
         */
        typedef lister*(*creator_fn)(callback_fn cb, void *ctx, void *type);
        
        /**
         * Creator functor, allows filelister objects to be created with a
         * specific value for type.
         */
        struct creator {
            void *type;
            creator_fn create;
            
            creator() = default;
            
            creator(creator_fn create) : create(create) {}
            creator(creator_fn create, void *type) : create(create), type(type) {}
            
            lister *operator()(callback_fn callback, void *ctx) {
                return create(callback, ctx, type);
            }
        };
        
        /**
         * A context pointer for use by the controller of this object.
         * This field is not used by the lister object.
         */
        void *context;
        /**
         * Callback function pointer.
         */
        callback_fn callback;
        
        
        /**
         * Begin listing the directory at the path, in a background
         * operation.
         *
         * path: The path of the directory.
         */
        void begin(const path_str &path);
        /**
         * Begin listing the directory with a particular file descriptor.
         * This may not be supported by all listers. Use the reads_fd
         * method to determine whether this method is supported.
         *
         * fd:    The open file descriptor of the directory.
         * dupfd: If true the file descriptor is duplicated before being
         *        used, otherwise 'fd' is used directly and closed after
         *        the operation completes.
         *
         * Returns true if reading directly from a file descriptor is
         * supported and hence a background operation was initiated.
         */
        bool begin(int fd, bool dupfd);
        
        /**
         * Cancels the operation.
         *
         * Once the callback is called with stage = FINISH, the callback
         * will not be called again, thus the operation can be considered
         * cancelled.
         */
        void cancel();        
        
        /**
         * Decrements the reference count.
         *
         * If the reference is decreased to zero the object is
         * deallocated.  This method should only be externally called
         * once, when the external controller no longer needs a reference
         * to the object.
         */
        void release();
        
        /**
         * Returns the error code of the last error.
         */
        int error() const {
            return err;
        }
        
        /**
         * Returns the file descriptor of the directory being listed.
         *
         * If this is not supported, reads_fd() returns false, -1 is
         * returned.
         */
        virtual int fd() const {
            return -1;
        }
        
        /**
         * Returns true if the lister supports reading directly from a
         * file descriptor i.e. begin(int fd, bool dupfd) can be used.  If
         * the return value is false only the begin(path_str path) method
         * can be used.
         */
        virtual bool reads_fd() const {
            return true;
        }
        
    protected:
    
        /** Error code of the last error. */
        int err;
        
        /**
         * Constructor.
         *
         * callback: The callback function pointer.
         * ctx: The value of the ctx pointer.
         *
         * The constructor is private in order to prevent the object from
         * being stack allocated. Each subclass has a static creation
         * method.
         */
        lister(callback_fn callback, void *ctx);
        
        /**
         * Destructor. Private in order to prevent the object from being
         * deallocated externally. The release method should be called
         * when the object is no longer needed.
         */
        virtual ~lister() = default;
        
        /**
         * Initializes the operation, called on a background thread.
         *
         * path: The path to the directory.
         *
         * Returns zero if successful, non-zero if there was an error, in
         * which case the operation finishes early.
         */
        virtual int init(const path_str &path) = 0;
        /**
         * Initializes the operation, called on a background thread.
         *
         * fd:    The file descriptor of the directory.
         * dupfd: True if the file descriptor should be duplicated.
         *
         * This method is meant to be overriden, if the lister supports
         * reading from a file descriptor.
         *
         * Returns zero if successful, non-zero if there was an error, in
         * which case the operation finishes early.
         */
        virtual int init(int fd, bool dupfd) {
            return 0;
        };
        
        /**
         * Reads the directory list, called on a background thread.
         */
        virtual void read_async() = 0;        

        /**
         * Calls the callback with stage = FINISH.
         * Sets err to error.
         */
        void call_finish(int error);
        /**
         * Callls the callback.
         *
         * stage:   The stage of the operation.
         * ent:     The entry struct of the entry being read.
         * st:      The stat attributes of the entry being read.
         *
         * Returns -1 if the operation was cancelled before or while
         * calling the callback.
         */
        int call_callback(stage_t stage, const entry *ent, const struct stat *st);
    };
}

#endif // NUC_LISTER_H
