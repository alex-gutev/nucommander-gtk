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

#include "file_model_columns.h"

#include "settings/app_settings.h"

using namespace nuc;

static void init_columns(std::vector<column_descriptor*> &columns, file_model_columns &model) {
    for (auto &name : app_settings::instance().columns()) {
        if (auto col = get_column(name)) {
            col->add_column(model);
            columns.push_back(col);
        }
    }
}

file_model_columns::file_model_columns() {
    add(ent);
    add(marked);
    add(score);
    add(color);

    init_columns(columns, *this);
}


file_model_columns &file_model_columns::instance() {
    static file_model_columns inst;
    return inst;
}
