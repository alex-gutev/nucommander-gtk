/*
 * NuCommander
 * Copyright (C) 2019  Alexander Gutev <alex.gutev@gmail.com>
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

#ifndef NUC_COMMANDS_CUSTOM_COMMANDS_H
#define NUC_COMMANDS_CUSTOM_COMMANDS_H

#include "commands.h"

namespace nuc {
    /**
     * Add the custom commands, retrieved from settings. to the
     * command table @a table.
     *
     * @param table The command table to add the custom commands to.
     */
    void add_custom_commands(std::unordered_map<std::string, std::unique_ptr<command>> &table);
}  // nuc


#endif /* NUC_COMMANDS_CUSTOM_COMMANDS_H */

// Local Variables:
// mode: c++
// End:
