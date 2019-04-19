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

#include "interface/file_view.h"

#include <algorithm>
#include <functional>

#include <gdk/gdkkeysyms.h>


#include "tasks/async_task.h"

#include "file_list/columns.h"
#include "file_list/filtered_list_controller.h"
#include "search/fuzzy_filter.h"

#include "settings/app_settings.h"


using namespace nuc;


//// Constructor

file_view::file_view(BaseObjectType *cobject, Glib::RefPtr<Gtk::Builder> & builder)
    : Gtk::Frame(cobject) {
    builder->get_widget("path_entry", path_entry);
    builder->get_widget("file_list", file_list_view);
    builder->get_widget("scroll_window", scroll_window);
    builder->get_widget("filter_entry", filter_entry);

    init_file_list();
    init_path_entry();
    init_filter_entry();

    // Exclude entry widget from tab order focus chain.
    set_focus_chain({file_list_view});
}


//// Initialization

void file_view::init_file_list() {
    init_columns();
    init_scroll_adjustments();
    init_file_list_events();
}

void file_view::init_columns() {
    for (auto &name : app_settings::instance().columns()) {
        if (auto column = get_column(name))
            file_list_view->append_column(*column->create());
    }
}

void file_view::init_scroll_adjustments() {
    // Create a seperate vertical adjustment object for the tree view
    // in order to disable "smooth scrolling" when navigating using
    // the arrow keys.

    auto adj = Gtk::Adjustment::create(0,0,0);

    // Signal handlers are added to the change events of both
    // adjustment objects in order to propagate the changes to the
    // other adjustment object. Currently GTK compares the new values
    // set to the previous values and fires change signals only if
    // they are different, thus no flag is needed to determine which
    // adjustment object fired the initial change signal.

    adj->signal_changed().connect([this] {
        auto adj = file_list_view->get_vadjustment();
        scroll_window->get_vadjustment()->configure(
            adj->get_value(),
            adj->get_lower(),
            adj->get_upper(),
            adj->get_step_increment(),
            adj->get_page_increment(),
            adj->get_page_size()
        );
    });

    adj->signal_value_changed().connect([this] {
        scroll_window->get_vadjustment()->set_value(file_list_view->get_vadjustment()->get_value());
    });

    file_list_view->set_vadjustment(adj);

    // Scroll window adjustment signals

    scroll_window->get_vadjustment()->signal_value_changed().connect([=] {
        file_list_view->get_vadjustment()->set_value(scroll_window->get_vadjustment()->get_value());
    });
}

void file_view::init_file_list_events() {
    // Add tree view signal handlers

    file_list_view->signal_row_activated().connect(sigc::mem_fun(this, &file_view::on_row_activate));

    // Add X Events

    file_list_view->add_events(Gdk::KEY_PRESS_MASK | Gdk::FOCUS_CHANGE_MASK);

    // Add classes for indicating focus and focus event handlers

    file_list_view->get_style_context()->add_class("file-list-unfocus");

    file_list_view->signal_focus_in_event().connect([=] (GdkEventFocus *e) {
        file_list_view->get_style_context()->remove_class("file-list-unfocus");
        return false;
    });

    file_list_view->signal_focus_out_event().connect([=] (GdkEventFocus *e) {
        file_list_view->get_style_context()->add_class("file-list-unfocus");
        return false;
    });

    // Add Event Signal Handlers

    file_list_view->get_selection()->signal_changed().connect(sigc::mem_fun(this, &file_view::on_selection_changed));
    file_list_view->signal_key_press_event().connect(sigc::mem_fun(this, &file_view::on_file_list_keypress), false);
}


void file_view::init_path_entry() {
    path_entry->signal_activate().connect(sigc::mem_fun(this, &file_view::on_path_entry_activate));
}

void file_view::init_filter_entry() {
    filter_entry->signal_changed().connect(sigc::mem_fun(*this, &file_view::filter_changed));

    filter_entry->signal_key_press_event().connect(sigc::mem_fun(*this, &file_view::filter_key_press_before), false);
    filter_entry->signal_key_press_event().connect(sigc::mem_fun(*this, &file_view::filter_key_press));
}


//// Changing the File List

void file_view::file_list(std::shared_ptr<file_list_controller> new_flist, bool push_old) {
    if (flist) {
        if (push_old)
            flist_stack.emplace_back(flist);

        signals.path.disconnect();
        signals.model_change.disconnect();
        signals.select_row.disconnect();
    }

    if (new_flist) {
        // Temporarily set flist to NULL so as to not forward the
        // selection change event to the old flist.
        flist = nullptr;
        filtered_list = nullptr;

        change_model(new_flist->list());
        select_row(new_flist->selected());

        signals.path = new_flist->signal_path().connect(sigc::mem_fun(*this, &file_view::on_path_changed));
        connect_model_signals(new_flist);

        path_entry->set_text(new_flist->path().path());
    }

    flist = new_flist;
    filtered_list = flist;

    m_filtering = false;
    filter_entry->hide();
}

std::shared_ptr<file_list_controller> file_view::pop_file_list() {
    auto it = flist_stack.end(), begin = flist_stack.begin();

    while (it != begin) {
        --it;
        auto flist = it->lock();

        it = flist_stack.erase(it);

        if (flist) return flist;
    }

    return nullptr;
}


//// Signal Handlers

void file_view::on_path_entry_activate() {
    flist->path(pathname(path_entry->get_text()));
    file_list_view->grab_focus();
}

void file_view::on_row_activate(const Gtk::TreeModel::Path &row_path, Gtk::TreeViewColumn* column) {
    auto row = *file_list_view->get_model()->get_iter(row_path);
    dir_entry &ent = *row[file_model_columns::instance().ent];

    m_signal_activate_entry.emit(this, flist.get(), &ent);
}

void file_view::on_selection_changed() {
    if (filtered_list) {
        auto row = file_list_view->get_selection()->get_selected();
        auto model = file_list_view->get_model();

        if (mark_rows) {
            auto prev_index = model->get_path(filtered_list->selected())[0];
            auto row_index = model->get_path(row)[0];

            size_t start, end;

            if (row_index > prev_index) {
                start = prev_index;
                end = row_index - mark_end_offset;
            }
            else {
                start = row_index + mark_end_offset;
                end = prev_index;
            }

            for (size_t i = start; i <= end; i++) {
                filtered_list->mark_row(model->children()[i]);
            }

            mark_rows = false;
        }

        if (row) {
            filtered_list->on_selection_changed(*row);
        }
    }
}

bool file_view::on_file_list_keypress(GdkEventKey *e) {
    if (filtering() && e->keyval == GDK_KEY_Escape) {
        end_filter();
        return true;
    }

    switch (e->keyval) {
    case GDK_KEY_Return:
        if (auto row = *file_list_view->get_selection()->get_selected()) {
            // Emit activate signal. This should be emitted automatically
            // by the widget however the signal is not emitted if the
            // selection was changed programmatically.
            file_list_view->row_activated(file_list_view->get_model()->get_path(row), *file_list_view->get_column(0));
        }
        return true;

    case GDK_KEY_Up:
    case GDK_KEY_Down:
        keypress_arrow(e);
        break;

    case GDK_KEY_Home:
    case GDK_KEY_End:
        keypress_change_selection(e, true);
        break;

    case GDK_KEY_Page_Down:
    case GDK_KEY_Page_Up:
        keypress_change_selection(e, false);
        break;
    }

    return false;
}

void file_view::keypress_arrow(const GdkEventKey *e) {
    if ((e->state & gtk_accelerator_get_default_mod_mask()) == GDK_SHIFT_MASK) {
        if (auto row = file_list_view->get_selection()->get_selected()) {
            filtered_list->mark_row(*row);
        }
    }
}

void file_view::keypress_change_selection(const GdkEventKey *e, bool mark_selected) {
    if ((e->state & gtk_accelerator_get_default_mod_mask()) == GDK_SHIFT_MASK) {
        mark_rows = true;
        mark_end_offset = mark_selected ? 0 : 1;
    }
}


void file_view::connect_model_signals(const std::shared_ptr<list_controller> &list) {
    signals.model_change = list->signal_change_model().connect(sigc::mem_fun(*this, &file_view::change_model));
    signals.select_row = list->signal_select().connect(sigc::mem_fun(*this, &file_view::select_row));
}

void file_view::on_path_changed(const pathname &path) {
    end_filter();
    entry_path(path);
}

void file_view::change_model(Glib::RefPtr<Gtk::ListStore> model) {
    file_list_view->set_model(model);
}

void file_view::select_row(Gtk::TreeRow row) {
    if (row) {
        file_list_view->get_selection()->select(row);
        file_list_view->scroll_to_row(file_list_view->get_model()->get_path(row));
    }
}


//// Changing the current path

void file_view::path(const pathname &path, bool move_to_old) {
    end_filter();

    entry_path(path);
    flist->path(path, move_to_old);
}

void file_view::entry_path(const std::string &path) {
    path_entry->set_text(path);
}


//// Getting Selected Entry

dir_entry *file_view::selected_entry() {
    auto row = filtered_list->selected();

    if (row)
        return row[file_model_columns::instance().ent];

    return nullptr;
}

std::vector<dir_entry*> file_view::selected_entries() const {
    return filtered_list ? filtered_list->selected_entries() : std::vector<dir_entry*>();
}


//// Directory VFS

std::shared_ptr<nuc::vfs> file_view::dir_vfs() const {
    return flist ? flist->dir_vfs() : nullptr;
}


//// Changing Keyboard Focus

void file_view::focus_path() {
    path_entry->grab_focus();
}


//// Filtering

void file_view::make_filter_model() {
    auto filter_list = filtered_list_controller::create(flist, [this] (Gtk::TreeRow row) {
        dir_entry *ent = row[file_model_columns::instance().ent];
        return fuzzy_match(Glib::ustring(ent->file_name()), filter_entry->get_text());
    });

    signals.model_change.disconnect();
    signals.select_row.disconnect();

    signals.select_row = filter_list->signal_select().connect(sigc::mem_fun(this, &file_view::select_row));

    filter_list->refilter();
    filtered_list = filter_list;

    file_list_view->set_model(filter_list->list());
    select_row(filter_list->selected());
}

void file_view::begin_filter() {
    filter_entry->show();
    filter_entry->grab_focus_without_selecting();

    if (!filtering()) {
        filter_entry->set_text("");

        make_filter_model();
        m_filtering = true;
    }
}

void file_view::begin_filter(const Glib::ustring &str) {
    begin_filter();

    filter_entry->set_text(str);
    filter_entry->set_position(str.length());
}


bool file_view::filtering() const {
    return m_filtering;
}

void file_view::end_filter() {
    if (filtering()) {
        auto selected_row = *file_list_view->get_selection()->get_selected();
        dir_entry *ent = selected_row ? (dir_entry*)selected_row[file_model_columns::instance().ent] : nullptr;

        filter_entry->hide();
        file_list_view->grab_focus();

        m_filtering = false;

        filtered_list = flist;
        file_list_view->set_model(flist->list());

        if (ent) {
            select_row(ent->context.row);
        }

        connect_model_signals(flist);
    }
}

void file_view::filter_changed() {
    auto filter_list = std::dynamic_pointer_cast<filtered_list_controller>(filtered_list);
    if (filter_list) filter_list->refilter();
}

bool file_view::filter_key_press(GdkEventKey *e) {
    file_list_view->grab_focus();
    bool stop = file_list_view->event((GdkEvent*)e);

    if (filtering()) filter_entry->grab_focus_without_selecting();

    return stop;
}

bool file_view::filter_key_press_before(GdkEventKey *e) {
    if (e->keyval == GDK_KEY_Return) {
        return filter_key_press(e);
    }

    return false;
}
