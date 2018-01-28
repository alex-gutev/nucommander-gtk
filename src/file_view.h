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

#ifndef NUC_FILE_VIEW_H
#define NUC_FILE_VIEW_H

#include <glibmm.h>

#include <gtkmm/builder.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#include "file_model_columns.h"
#include "vfs.h"

namespace nuc {
    class file_view : public Gtk::Frame {
        Gtk::Entry *path_entry;
        Gtk::TreeView *file_list;

        Glib::RefPtr<Gtk::ListStore> list_store;
        file_model_columns columns;
        
        nuc::vfs vfs;
        
        void init_file_list();
        void init_model();
        
        void init_path_entry();
        
        void init_vfs();
        
        void read_path(const std::string &path);
        void entry_path(const std::string &path);
        
        void vfs_callback(nuc::vfs::op_stage stage);
        void begin_read();
        void finish_read();
        
        void add_row(dir_entry &ent);
        
        void path_entry_activate();
        
    public:
        file_view(BaseObjectType *cobject, Glib::RefPtr<Gtk::Builder> &builder);
        
        void path(const std::string &path);
    };
}

#endif // NUC_FILE_VIEW_H
