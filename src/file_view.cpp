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

#include "async_task.h"

using namespace nuc;

file_view::file_view(BaseObjectType *cobject, Glib::RefPtr<Gtk::Builder> & builder)
    : Gtk::Frame(cobject) {
    builder->get_widget("path_entry", path_entry);
    builder->get_widget("file_list", file_list);
    
    init_vfs();
    init_file_list();
    init_path_entry();

    // Exclude entry widget from tab focus chain
    set_focus_chain({file_list});;
}

void file_view::init_file_list() {
    init_model();
}

void file_view::init_model() {
    list_store = Gtk::ListStore::create(columns);
    
    file_list->set_model(list_store);
    file_list->append_column("Name", columns.name);
    
    auto column = file_list->get_column(0);
    column->set_sort_column(columns.name);

    // Set "Name" as the default sort column
    list_store->set_sort_column(0, Gtk::SortType::SORT_ASCENDING);
}

void file_view::init_vfs() {
    vfs.callback = [=] (nuc::vfs &, nuc::vfs::op_stage stage) {
        dispatch_main([=] {
            vfs_callback(stage);
        });
    };
}

void file_view::init_path_entry() {
    path_entry->signal_activate().connect(sigc::mem_fun(this, &file_view::on_path_entry_activate));
}


void file_view::vfs_callback(vfs::op_stage stage) {
    switch (stage) {
        case nuc::vfs::BEGIN:
            begin_read();
            break;
            
        case nuc::vfs::CANCELLED:
            finish_read();
            break;
            
        case nuc::vfs::FINISH:
            finish_read();
            add_new_rows();
            break;
            
        case nuc::vfs::ERROR:
            break;
    }
}

void file_view::begin_read() {
    list_store->clear();
}

void file_view::finish_read() {
    vfs.free_op();
}

void file_view::add_new_rows() {
    vfs.commit_read();
    vfs.for_each([=] (dir_entry &ent) {
        add_row(ent);
    });
    
    auto row = list_store->children()[0];
    
    if (row)
        file_list->get_selection()->select(row);
}

void file_view::add_row(dir_entry &ent) {
    auto row = *list_store->append();
    row[columns.name] = ent.file_name();
}



void file_view::path(const std::string &path) {
    entry_path(path);
    read_path(path);
}

void file_view::entry_path(const std::string &path) {
    path_entry->set_text(path);
}

void file_view::read_path(const std::string& path) {
    vfs.read(path);
}


void file_view::on_path_entry_activate() {
    read_path(path_entry->get_text());
    file_list->grab_focus();
}
