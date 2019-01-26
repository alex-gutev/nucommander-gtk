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

#ifndef NUC_FILE_LIST_SORT_FUNC
#define NUC_FILE_LIST_SORT_FUNC

#include <glibmm.h>

#include <gtkmm/liststore.h>

#include "directory/dir_entry.h"
#include "file_model_columns.h"

/**
 * File List Sorting Functions.
 */

namespace nuc  {
    /* Sort Functions */

    /**
     * Sort by entry type. Orders directories before other files.
     */
    int sort_entry_type(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b);

    /**
     * Sort by file name.
     */
    int sort_name(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b);

    /**
     * Sort by file size.
     */
    int sort_size(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b);


    /* Sort Utilities */

    /**
     * Sort function, wrapping another sort function, that is
     * invariant to the sort order, that is the function always sorts
     * in the same order regardless of whether the order is ascending
     * or descending
     */
    template <typename F>
    struct invariant_sort {
        /**
         * Current sort order, 1 - ascending, -1 - descending.
         */
        const int order = 1;

        /**
         * Wrapped sort function.
         */
        F f;

        /** Constructor */
        invariant_sort(F f, int order) : order(order), f(f) {}

        int operator()(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b) {
            return order * f(a,b);
        }
    };

    /**
     * Creates a sort function, wrapping the function @a f which is
     * invariant to the current sort order.
     *
     * @param f The sort function.
     *
     * @param order The current sort order, 1 - ascending, 0 -
     *   descending.
     *
     * @return The order-invariant sort function.
     */
    template <typename F>
    invariant_sort<F> make_invariant_sort(F f, Gtk::SortType order) {
        return invariant_sort<F>(f, order == Gtk::SortType::SORT_ASCENDING ? 1 : -1);
    }


    /** Combining Sort Functions */

    /**
     * Sort function that is a combination of multiple sort functions.
     *
     * The functions sorts two entries a and b in the order of the
     * first sort function, by which they do not compare equal.
     */
    template <typename ...Fs>
    struct combined_sort {
        int operator()(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b) {
            return 0;
        }
    };

    template <typename F1, typename ...Fs>
    struct combined_sort<F1, Fs...> {
        F1 f1;
        combined_sort<Fs...> f2;

        combined_sort(F1 f1, Fs... fs) : f1(f1), f2(fs...) {}

        int operator()(const Gtk::TreeModel::iterator &a, const Gtk::TreeModel::iterator &b) {
            if (int order = f1(a, b)) {
                return order;
            }

            return f2(a,b);
        }
    };

    /**
     * Combines the sort functions @a fs into one sort function.
     *
     * The functions sorts two entries a and b in the order of the
     * first sort function, of @a fs, by which they do not compare
     * equal.
     *
     * @param fs The sort functions.
     * @return The combined sort function.
     */
    template <typename ... Fs>
    combined_sort<Fs...> combine_sort(Fs... fs) {
        return combined_sort<Fs...>(fs...);
    }
}

#endif

// Local Variables:
// mode: c++
// End:
