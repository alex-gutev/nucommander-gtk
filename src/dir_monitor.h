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

#ifndef NUC_DIR_MONITOR_H
#define NUC_DIR_MONITOR_H

#include <giomm.h>
#include <glibmm.h>

#include <sigc++/sigc++.h>

#include <utility>
#include <deque>

#include <cassert>

#include "types.h"

namespace nuc {
    /**
     * Monitors a directory for changes and emits events for each
     * change.
     */
    class dir_monitor {
        /**
         * Interval in milliseconds, during which if no event is
         * received, the callback is called with EVENTS_END.
         */
        unsigned int interval = 2000;
        
    public:
        /**
         * Event type.
         */
        enum event_type {
            /**
             * Events regarding individual files in the directory.
             * Each event contains the file name of the affected
             * file. For renamed events (FILE_RENAMED) 2 file names
             * are given, the original name and the new name.
             *
             * These events are only supported for regular
             * directories.
             */
            FILE_CREATED = 0,
            FILE_MODIFIED,
            FILE_DELETED,
            FILE_RENAMED,

            /**
             * File events on the directory itself. A DIR_MODIFIED
             * event is sent if individual file events are not
             * supported for the directory type.
             */

            /**
             * Event indicating that the directory was modified. This
             * event is sent if individual file events are not
             * supported for the diirectory type. The only way to
             * obtain the details of which file was
             * created/modified/deleted is to rescan the entire
             * directory.
             */
            DIR_MODIFIED,
            DIR_DELETED,

            /**
             * Sent before the first event in a block of events.
             */
            EVENTS_BEGIN,
            /**
             * Sent after the time interval elapses after the last
             * event. The next event will be preceeded by an
             * EVENTS_BEGIN event.
             *
             * The purpose of this event and EVENTS_BEGIN is to group
             * closely related changes into blocks.
             */
            EVENTS_END
        };

        /**
         * Event structure. Contains details of the event.
         */
        class event {
            /**
             * Event type - One of the event_type constants.
             */
            event_type m_type;

            /**
             * The file name of the file which generated the
             * rename. This field is not used in EVENTS_BEGIN and
             * EVENTS_END.
             *
             * For rename events, this contains the original file
             * name.
             */
            path_str file;
            /**
             * Used only in rename events. The new file name of the
             * renamed file.
             */
            path_str other_file;

        public:

            /** Constructors */
            
            event(event_type type) : m_type(type) {}

            template <typename Arg1, typename Arg2>
            event(event_type type, Arg1&& file, Arg2&& other_file) :
                m_type(type), file(std::forward<Arg1>(file)), other_file(std::forward<Arg2>(other_file)) {}


            /* Accessors */
            
            const event_type type() const {
                return m_type;
            }

            /**
             * Returns the file name of the file which generated the
             * event. For rename events (FILE_RENAMED), this is the
             * original file name.
             */
            const path_str &src() const {
                assert(m_type != EVENTS_BEGIN && m_type != EVENTS_END);
                return file;
            }
            /**
             * Returns the new file name of the renamed file. Should
             * only be used when the event is FILE_RENAMED.
             */
            const path_str &dest() const {
                assert(m_type == FILE_RENAMED);
                return other_file;
            }
        };

        /**
         * Event signal type. Takes one argument of type
         * dir_monitor::event.
         */
        typedef sigc::signal<void, event> event_signal_type;

        /**
         * Event signal.
         */
        event_signal_type signal_event();
        
        /**
         * Begins monitoring a directory.
         *
         * @param path   The path to the directory
         *
         * @param paused If true the monitor is initially paused,
         *               resume has to be called in order to begin
         *               emitting the event signal.
         *
         * @return Returns true if the monitoring began successfully.
         */
        bool monitor_dir(const path_str &path, bool paused = true);

        /**
         * Cancels the monitor, if the monitor is paused all queued
         * events are discarded.
         */
        void cancel();

        /**
         * Pauses the monitor. A paused monitor still receives events
         * however these events are stored in a queue rather than
         * emitting the event signal.
         */
        void pause();
        /**
         * Resumes the monitor. The event signal is first emitted for
         * the events stored in the queue (since the monitor was
         * paused), after which it is emitted for new events.
         */
        void resume();

        
        /**
         * Returns the event block timer interval.
         */
        unsigned int timeout() const {
            return interval;
        };
        /**
         * Sets the event block timer interval.
         */
        void timeout(unsigned int time) {
            interval = time;
        }
        
        
    private:
        /**
         * Directory being monitored.
         */
        Glib::RefPtr<Gio::File> dir_file;
        
        /**
         * Monitors the directory for changes.
         */
        Glib::RefPtr<Gio::FileMonitor> monitor;
        /**
         * Monitor timer connection object.
         */
        sigc::connection timer;        

        /**
         * Event signal.
         */
        event_signal_type m_signal_event;

        /**
         * Queue where events are stored when the monitor is paused.
         */
        std::deque<event> event_queue;
        
        
        /**
         * True if the directory is currently being modified, that is
         * the timer started since the last event has not yet elapsed.
         */
        bool changing = false;

        /**
         * True if the monitor is paused.
         */
        bool paused = true;
        
        /**
         * File system event signal handler.
         */
        void on_file_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent type);
        /**
         * Timer interval elapsed signal handler.
         */
        bool on_timer_elapsed();

        /**
         * Returns true if the event applies to the directory file
         * itself.
         */
        bool is_dir_event(const Glib::RefPtr<Gio::File> &file);        
        
        /**
         * Creates the timer
         */
        void create_timer();
        
        /**
         * Stops the current timer if any.
         */
        void stop_timer();

        /**
         * Calls the event signal handler.
         */
        void emit_event(event_type type, path_str file = path_str(), path_str other_file = path_str());
    };
}

#endif // NUC_DIR_MONITOR_H

// Local Variables:
// mode: c++
// End:
