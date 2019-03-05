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

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE dir_tree

#include <boost/test/unit_test.hpp>

#include <string>
#include <iostream>

#include "directory/dir_tree.h"
#include "directory/archive_tree.h"

using namespace nuc;


BOOST_AUTO_TEST_SUITE(flat_dir_tree_tests)

BOOST_AUTO_TEST_CASE(add_entry) {
    dir_tree tree;

    lister::entry ent;
    ent.name = "foo.txt";
    ent.type = DT_LNK;

    struct stat st{};
    st.st_mode = S_IFREG | S_IRWXU;

    dir_entry *foo = tree.add_entry(ent, st);
    dir_entry *bar = tree.add_entry(dir_entry("bar.x", dir_entry::type_reg));

    BOOST_CHECK_EQUAL(foo->subpath().path(), "foo.txt");
    BOOST_CHECK_EQUAL(foo->type(), dir_entry::type_reg);
    BOOST_CHECK_EQUAL(foo->ent_type(), dir_entry::type_lnk);
    BOOST_CHECK_EQUAL(foo->attr().st_mode & ~S_IFMT, S_IRWXU);

    BOOST_CHECK_EQUAL(tree.get_entry("foo.txt"), foo);

    BOOST_CHECK_EQUAL(bar->subpath().path(), "bar.x");
    BOOST_CHECK_EQUAL(bar->type(), dir_entry::type_reg);
    BOOST_CHECK_EQUAL(bar->ent_type(), dir_entry::type_reg);
    BOOST_CHECK_EQUAL(tree.get_entry("bar.x"), bar);
}

BOOST_AUTO_TEST_CASE(subdirectories) {
    // Flat directory trees don't have subdirectories so simply check
    // that any attempt to switch to a subdirectory fails.

    dir_tree tree;
    dir_entry *ent = tree.add_entry(dir_entry("foo", dir_entry::type_dir));

    BOOST_CHECK_EQUAL(tree.subpath().path(), "");

    tree.subpath("foo");
    BOOST_CHECK_EQUAL(tree.subpath().path(), "");

    tree.subpath("bar");
    BOOST_CHECK_EQUAL(tree.subpath().path(), "");

    BOOST_CHECK_EQUAL(tree.subpath_dir("foo"), (dir_tree::dir_map *)nullptr);
    BOOST_CHECK_EQUAL(tree.subpath_dir("bar"), (dir_tree::dir_map *)nullptr);

    BOOST_CHECK(!tree.is_subdir(*ent));
    BOOST_CHECK(tree.at_basedir());
}

BOOST_AUTO_TEST_CASE(get_entry) {
    dir_tree tree;

    dir_entry *foo = tree.add_entry(dir_entry("foo", dir_entry::type_dir));
    dir_entry *bar = tree.add_entry(dir_entry("bar.txt", dir_entry::type_reg));

    BOOST_CHECK_EQUAL(tree.get_entry("foo"), foo);
    BOOST_CHECK_EQUAL(tree.get_entry("bar.txt"), bar);
    BOOST_CHECK_EQUAL(tree.get_entry("baz"), (dir_entry *)nullptr);

    auto its = tree.get_entries("foo");

    BOOST_REQUIRE(its.first != its.second);
    BOOST_CHECK_EQUAL(&(its.first->second), foo);
    BOOST_CHECK(++its.first == its.second);
}

BOOST_AUTO_TEST_SUITE_END()
