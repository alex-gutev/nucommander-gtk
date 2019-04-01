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

#include "dir_monitor.h"

#include "settings/app_settings.h"

using namespace nuc;

dir_monitor::event_signal_type dir_monitor::signal_event() {
    return m_signal_event;
}


bool dir_monitor::monitor_dir(const paths::string &path, bool pause, bool is_dir) {
    cancel();

    paused = pause;
    dir_events = is_dir;

    changing = false;

    dir_file = Gio::File::create_for_path(path);

    if (is_dir) {
        monitor = dir_file->monitor_directory(Gio::FILE_MONITOR_WATCH_MOVES | Gio::FILE_MONITOR_WATCH_MOUNTS);
    }
    else {
        monitor = dir_file->monitor(Gio::FILE_MONITOR_WATCH_MOVES);
    }


    if (monitor) {
        monitor->signal_changed().connect(sigc::mem_fun(this, &dir_monitor::on_file_changed));
        return true;
    }

    return false;
}

void dir_monitor::cancel() {
    if (monitor) {
        event_queue.clear();
        monitor->cancel();

        monitor = Glib::RefPtr<Gio::FileMonitor>(nullptr);

        stop_timer();
        dir_file = Glib::RefPtr<Gio::File>(nullptr);
    }
}

void dir_monitor::pause() {
    paused = true;
}

void dir_monitor::resume() {
    if (paused) {
        paused = false;

        while (!event_queue.empty()) {
            event e = std::move(event_queue.front());
            event_queue.pop_front();

            m_signal_event.emit(std::move(e));
        }
    }
}


void dir_monitor::on_file_changed(const Glib::RefPtr<Gio::File> &file, const Glib::RefPtr<Gio::File> &other_file, Gio::FileMonitorEvent type) {
    if (dir_events) {
        create_timer();

        if (!changing) {
            emit_event(EVENTS_BEGIN);
            changing = true;
        }
    }

    switch (type) {
    case Gio::FILE_MONITOR_EVENT_CREATED:
    case Gio::FILE_MONITOR_EVENT_MOVED_IN:
        emit_event(FILE_CREATED, file->get_path());
        break;

    case Gio::FILE_MONITOR_EVENT_DELETED:
    case Gio::FILE_MONITOR_EVENT_MOVED_OUT: {
        event_type type = FILE_DELETED;

        if (is_dir_event(file)) {
            type = DIR_DELETED;
            end_events();
        }

        emit_event(type, file->get_path());
    } break;

    case Gio::FILE_MONITOR_EVENT_RENAMED: {
        event_type type = FILE_RENAMED;

        if (other_file && is_dir_event(other_file)) {
            type = DIR_MODIFIED;
            end_events();
        }
        emit_event(type, file->get_path(), other_file->get_path());
    } break;

    case Gio::FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
    case Gio::FILE_MONITOR_EVENT_CHANGES_DONE_HINT: {
        event_type type = FILE_MODIFIED;

        if (is_dir_event(file)) {
            type = DIR_MODIFIED;
            end_events();
        }

        emit_event(type, file->get_path());
    } break;
    }
}

bool dir_monitor::is_dir_event(const Glib::RefPtr<Gio::File> & file) {
    return dir_file->equal(file);
}

void dir_monitor::end_events() {
    if (changing) {
        stop_timer();

        on_timer_elapsed();
    }
}

void dir_monitor::emit_event(event_type type, paths::string file, paths::string other_file) {
    if (!paused) {
        m_signal_event.emit(event(type, std::move(file), std::move(other_file)));
    }
    else {
        event_queue.emplace_back(type, std::move(file), std::move(other_file));
    }
}


bool dir_monitor::on_timer_elapsed() {
    changing = false;
    emit_event(EVENTS_END);

    return false;
}

void dir_monitor::create_timer() {
    stop_timer();

    // Can also use connect_seconds
    timer = Glib::signal_timeout().connect(sigc::mem_fun(this, &dir_monitor::on_timer_elapsed), app_settings::instance().dir_refresh_timeout());
}

void dir_monitor::stop_timer() {
    if (timer.connected()) {
        timer.disconnect();
    }
}
