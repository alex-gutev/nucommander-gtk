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

#ifndef NUC_LISTER_ARCHIVE_LISTER_H
#define NUC_LISTER_ARCHIVE_LISTER_H

#include "lister/lister.h"
#include "plugins/archive_plugin.h"

namespace nuc {
    /**
     * Archive lister subclass.
     *
     * Lists the contents of archives using an archive plugin.
     */
    class archive_lister : public lister {
    protected:
        /**
         * Pointer to archive plugin with which the archive is read.
         */
        archive_plugin *plugin;

        /**
         * Plugin-specific archive handle.
         */
        void *handle = nullptr;

    public:
        /**
         * Constructs an archive lister.
         *
         * @param plugin The plugin which reads the archive.
         * @param path Path to the archive file.
         */
        archive_lister(archive_plugin *plugin, const pathname::string &path);

        /**
         * Constructs an archive lister, however does not open any
         * archive file, that is handle is set to nullptr. This is
         * intended to be used only by subclasses.
         *
         * @param plugin The plugin for reading the archive.
         */
        archive_lister(archive_plugin *plugin);

        virtual ~archive_lister();

        /* Method Overrides */

        virtual void close();

        virtual bool read_entry(entry &ent);
        virtual bool entry_stat(struct stat &st);

        virtual instream *open_entry();

        /* Archive Lister Specific Methods */

        /**
         * Retrieves the symbolic link target path of the last entry
         * read if it is a symbolic link.
         *
         * @return The target path.
         */
        const char *symlink_path() const;

        /**
         * Returns the plugin-specific handle to the archive.
         *
         * @return Plugin specific handle
         */
        void *arch_handle() {
            return handle;
        }
    };
}

#endif // NUC_LISTER_ARCHIVE_LISTER_H

// Local Variables:
// mode: c++
// indent-tabs-mode: nil
// End:
