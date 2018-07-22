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

#ifndef NUC_ICON_LOADER_H
#define NUC_ICON_LOADER_H

#include <string>

#include <gdkmm/pixbuf.h>
#include <giomm/contenttype.h>

#include "dir_entry.h"

namespace nuc {
    /**
     * Loads icons for entries.
     */
    class icon_loader {
        /**
         * Returns the icon name for a file type.
         *
         * @param type The file type.
         *
         * @return The name of the icon for the file type.
         */
        std::string name_for_type(dir_entry::entry_type type);

        /**
         * Returns the icon for the file, with name @a name, based on
         * its mime-type guessed from its name.
         *
         * @param name The file name.
         *
         * @return The Gio::Icon corresponding to the mime-type of the
         *    file.
         */
        Glib::RefPtr<Gio::Icon> icon_from_name(const std::string &name);


    public:

        /**
         * Returns the singleton instance.
         */
        static icon_loader &instance();

        /**
         * Loads the icon for the entry @ent.
         *
         * @param ent The entry.
         *
         * @return The icon of the entry or a null RefPtr if no icon
         *    could be found for the entry.
         */
        Glib::RefPtr<Gdk::Pixbuf> load_icon(const dir_entry &ent);
    };
}

#endif // NUC_ICON_LOADER_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
