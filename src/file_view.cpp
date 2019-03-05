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

#include "file_view.h"

#include "tasks/async_task.h"
#include "file_list/columns.h"

#include "search/fuzzy_filter.h"

#include <gdk/gdkkeysyms.h>

#include <algorithm>
#include <functional>

// For debugging only
//#include <iostream>
//#include <stdio.h>

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
    // Name and Icon

    file_list_view->append_column(*file_column_descriptors[0]->create());

    // File Size

    file_list_view->append_column(*file_column_descriptors[1]->create());

    // Last Modified Date

    file_list_view->append_column(*file_column_descriptors[2]->create());
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

        flist->signal_change_model().clear();
        flist->signal_select().clear();
        flist->signal_path().clear();
    }

    if (new_flist) {
        // Temporarily set flist to NULL to prevent so as to not
        // forward the selection change event to the old flist.
        flist = nullptr;

        change_model(new_flist->list());
        select_row(new_flist->selected());

        new_flist->signal_path().connect(sigc::mem_fun(*this, &file_view::on_path_changed));
        new_flist->signal_change_model().connect(sigc::mem_fun(*this, &file_view::change_model));
        new_flist->signal_select().connect(sigc::mem_fun(*this, &file_view::select_row));

        path_entry->set_text(new_flist->path().path());
    }

    flist = new_flist;
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
    flist->path(path_entry->get_text());
    file_list_view->grab_focus();
}

void file_view::on_row_activate(const Gtk::TreeModel::Path &row_path, Gtk::TreeViewColumn* column) {
    auto row = filter_model->children()[row_path[0]];

    dir_entry &ent = *row[file_model_columns::instance().ent];

    m_signal_activate_entry.emit(this, flist.get(), &ent);
}

void file_view::on_selection_changed() {
    if (flist) {
        auto row = file_list_view->get_selection()->get_selected();

        if (row) {
            flist->on_selection_changed(*filter_model->convert_iter_to_child_iter(row));
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

    default:
        return flist ? flist->on_keypress(e) : false;
    }
}


void file_view::on_path_changed(const paths::pathname &path) {
    end_filter();
    entry_path(path);
}

void file_view::change_model(Glib::RefPtr<Gtk::ListStore> model) {
    make_filter_model(model);

    file_list_view->set_model(filter_model);
}

void file_view::select_row(Gtk::TreeRow row) {
    if (row) {
        auto it = filter_model->convert_child_iter_to_iter(row);

        if (it) {
            file_list_view->get_selection()->select(it);
            file_list_view->scroll_to_row(file_list_view->get_model()->get_path(it));
        }
    }
}


//// Changing the current path

void file_view::path(const paths::pathname &path, bool move_to_old) {
    end_filter();

    entry_path(path);
    flist->path(path, move_to_old);
}

void file_view::entry_path(const std::string &path) {
    path_entry->set_text(path);
}


//// Getting Selected Entry

dir_entry *file_view::selected_entry() {
    auto row = flist->selected();

    if (row)
        return row[file_model_columns::instance().ent];

    return nullptr;
}


//// Changing Keyboard Focus

void file_view::focus_path() {
    path_entry->grab_focus();
}


//// Filtering

void file_view::make_filter_model(Glib::RefPtr<Gtk::ListStore> model) {
    filter_model = Gtk::TreeModelFilter::create(model);
    filter_model->set_visible_func([this] (const Gtk::TreeModel::iterator &it) -> bool {
        if (m_filtering) {
            Glib::ustring name = (*it)[file_model_columns::instance().name];
            return fuzzy_match(name, filter_entry->get_text());
        }

        return true;
    });
}

void file_view::begin_filter() {
    filter_entry->show();
    filter_entry->grab_focus_without_selecting();

    if (!filtering()) {
        m_filtering = true;
    }
}

bool file_view::filtering() const {
    return m_filtering;
}

void file_view::end_filter() {
    if (filtering()) {
        filter_entry->hide();
        filter_entry->set_text("");
        file_list_view->grab_focus();

        m_filtering = false;
        filter_model->refilter();
    }
}

void file_view::filter_changed() {
    auto key = filter_entry->get_text();

    filter_model->refilter();
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
