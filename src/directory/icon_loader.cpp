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

#include "icon_loader.h"

#include <gtkmm/icontheme.h>


using namespace nuc;

icon_loader & icon_loader::instance() {
    static icon_loader loader;
    return loader;
}

Glib::RefPtr<Gdk::Pixbuf> icon_loader::load_icon(const nuc::dir_entry &ent) {
    auto theme = Gtk::IconTheme::get_default();
    auto type = ent.type();

    Glib::RefPtr<Gdk::Pixbuf> icon;

    if (type != dir_entry::type_reg) {
        auto info = theme->lookup_icon(name_for_type(type), 16, Gtk::ICON_LOOKUP_FORCE_SIZE);
        if (info) icon = info.load_icon();
    }
    else {
        auto info = theme->lookup_icon(icon_from_name(ent.file_name()), 16, Gtk::ICON_LOOKUP_FORCE_SIZE);
        if (info) icon = info.load_icon();
    }

    if (!icon) {
        auto info = theme->lookup_icon("gtk-file", 16, Gtk::ICON_LOOKUP_FORCE_SIZE);
        if (info) icon = info.load_icon();
    }

    return icon;
}

std::string icon_loader::name_for_type(dir_entry::entry_type type) {
    switch (type) {
    case dir_entry::type_parent:
        return "go-up";

    case dir_entry::type_dir:
        return "folder";

    case dir_entry::type_fifo:
        return "inode-fifo";

    case dir_entry::type_blk:
        return "inode-blockdevice";

    case dir_entry::type_chr:
        return "inode-chardevice";

    case dir_entry::type_sock:
        return "inode-socket";

    default:
        return "inode-x-generic";
    }
}

Glib::RefPtr<Gio::Icon> icon_loader::icon_from_name(const std::string &name) {
    bool uncertain;
    return Gio::content_type_get_icon(Gio::content_type_guess(name, NULL, 0, uncertain));
}


// Local Variables:
// indent-tabs-mode: nil
// End:
