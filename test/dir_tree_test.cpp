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


BOOST_AUTO_TEST_SUITE(archive_dir_tree_tests)


BOOST_AUTO_TEST_SUITE(adding_entries)

BOOST_AUTO_TEST_CASE(add_entry) {
    archive_tree atree;
    dir_tree &tree = atree;

    lister::entry ent;
    ent.name = "foo.txt";
    ent.type = DT_LNK;

    struct stat st{};
    st.st_mode = S_IFREG | S_IRWXU;

    dir_entry *foo = tree.add_entry(ent, st);
    dir_entry *bar = tree.add_entry(dir_entry("bar.x", dir_entry::type_reg));
    dir_entry *foo_dir1 = tree.add_entry(dir_entry("foo/baz.txt", dir_entry::type_reg));
    dir_entry *foo_dir2 = tree.add_entry(dir_entry("foo/bar/../baz/./../bar.txt", dir_entry::type_reg));
    dir_entry *parent_dir = tree.add_entry(dir_entry(".././foo", dir_entry::type_dir));

    // Check foo.txt

    BOOST_CHECK_EQUAL(foo->subpath().path(), "foo.txt");
    BOOST_CHECK_EQUAL(foo->type(), dir_entry::type_reg);
    BOOST_CHECK_EQUAL(foo->ent_type(), dir_entry::type_lnk);
    BOOST_CHECK_EQUAL(foo->attr().st_mode & ~S_IFMT, S_IRWXU);
    BOOST_CHECK_EQUAL(tree.get_entry("foo.txt"), foo);

    // Check bar.x

    BOOST_CHECK_EQUAL(bar->subpath().path(), "bar.x");
    BOOST_CHECK_EQUAL(bar->type(), dir_entry::type_reg);
    BOOST_CHECK_EQUAL(bar->ent_type(), dir_entry::type_reg);
    BOOST_CHECK_EQUAL(tree.get_entry("bar.x"), bar);

    // Check foo directory

    BOOST_CHECK_EQUAL(foo_dir1->subpath().path(), "foo");
    BOOST_CHECK_EQUAL(foo_dir1->type(), dir_entry::type_dir);
    BOOST_CHECK_EQUAL(foo_dir1->ent_type(), dir_entry::type_dir);
    BOOST_CHECK_EQUAL(tree.get_entry("foo"), foo_dir1);

    // Check foo/baz.txt

    dir_entry *foo_baz = tree.get_entry("foo/baz.txt");
    BOOST_REQUIRE(foo_baz);
    BOOST_CHECK_EQUAL(foo_baz->subpath().path(), "foo/baz.txt");
    BOOST_CHECK_EQUAL(foo_baz->orig_subpath().path(), "foo/baz.txt");
    BOOST_CHECK_EQUAL(foo_baz->type(), dir_entry::type_reg);
    BOOST_CHECK_EQUAL(foo_baz->ent_type(), dir_entry::type_reg);

    // Check foo/bar.txt

    dir_entry *foo_bar = tree.get_entry("foo/bar.txt");
    BOOST_REQUIRE(foo_bar);
    BOOST_CHECK_EQUAL(foo_bar->subpath().path(), "foo/bar.txt");
    BOOST_CHECK_EQUAL(foo_bar->orig_subpath().path(), "foo/bar/../baz/./../bar.txt");
    BOOST_CHECK_EQUAL(foo_bar->type(), dir_entry::type_reg);
    BOOST_CHECK_EQUAL(foo_bar->ent_type(), dir_entry::type_reg);


    // Check that foo_dir2 is null as the foo directory has already
    // been returned once.
    BOOST_CHECK_EQUAL(foo_dir2, (dir_entry *)nullptr);


    // Check .. directory

    BOOST_CHECK_EQUAL(parent_dir->subpath().path(), "..");
    BOOST_CHECK_EQUAL(parent_dir->type(), dir_entry::type_dir);
    BOOST_CHECK_EQUAL(parent_dir->ent_type(), dir_entry::type_dir);
    BOOST_CHECK_EQUAL(tree.get_entry(".."), parent_dir);


    // Check ../foo

    dir_entry *foo2 = tree.get_entry("../foo");
    BOOST_CHECK_EQUAL(foo2->subpath().path(), "../foo");
    BOOST_CHECK_EQUAL(foo2->orig_subpath().path(), ".././foo");
    BOOST_CHECK_EQUAL(foo2->type(), dir_entry::type_dir);
    BOOST_CHECK_EQUAL(foo2->ent_type(), dir_entry::type_dir);
}

BOOST_AUTO_TEST_CASE(duplicates) {
    archive_tree atree;
    dir_tree &tree = atree;

    lister::entry ent;
    ent.name = "foo";
    ent.type = DT_LNK;

    struct stat st{};
    st.st_ino = 1;
    st.st_mode = S_IFREG | S_IRWXU;

    dir_entry *foo1 = tree.add_entry(ent, st);

    dir_entry *foo2 = tree.add_entry(dir_entry("foo", dir_entry::type_dir));

    st.st_ino = 2;
    st.st_mode = S_IFREG | S_IRUSR;
    dir_entry *foo3 = tree.add_entry(dir_entry("foo", st));

    auto ents = tree.get_entries("foo");
    BOOST_CHECK_EQUAL(std::distance(ents.first, ents.second), 3);

    for (auto it = ents.first; it != ents.second; ++it) {
        dir_entry *ent = &it->second;

        BOOST_CHECK_EQUAL(ent->subpath().path(), "foo");

        if (ent == foo1) {
            BOOST_CHECK_EQUAL(ent->ent_type(), dir_entry::type_lnk);
            BOOST_CHECK_EQUAL(ent->attr().st_ino, 1);
            BOOST_CHECK_EQUAL(ent->attr().st_mode, S_IFREG | S_IRWXU);

            // Set to null in order for test to fail if foo1 is
            // duplicated
            foo1 = nullptr;
        }
        else if (ent == foo2) {
            BOOST_CHECK_EQUAL(ent->type(), dir_entry::type_dir);

            // Set to null in order for test to fail if foo1 is
            // duplicated
            foo2 = nullptr;
        }
        else if (ent == foo3) {
            BOOST_CHECK_EQUAL(ent->attr().st_ino, 2);
            BOOST_CHECK_EQUAL(ent->attr().st_mode, S_IFREG | S_IRUSR);

            // Set to null in order for test to fail if foo1 is
            // duplicated
            foo3 = nullptr;
        }
        else {
            BOOST_FAIL("Entry, returned by get_entry(), not one of the three added.");
        }
    }
}

BOOST_AUTO_TEST_CASE(directories) {
    // Test that directory entries are added correctly:
    //
    // When an entry which contains a directory component in its name,
    // e.g. dir/file, is added to the tree, a directory entry (for the
    // directory component) is automatically created.
    //
    // The following tests test that directory entries explicitly
    // added to the tree always replace automatically added entries
    // and that automatically added entries never replace directory
    // entries which were explicitly added.

    archive_tree atree;
    dir_tree &tree = atree;

    tree.add_entry(dir_entry("foo/bar.txt", dir_entry::type_reg));

    // Add foo directory entry
    {
        lister::entry ent;
        struct stat st{};

        ent.name = "foo";
        ent.type = DT_DIR;

        st.st_ino = 100;

        tree.add_entry(ent, st);
    }

    // Add bar directory entry
    {
        lister::entry ent;
        struct stat st{};

        ent.name = "bar";
        ent.type = DT_DIR;

        st.st_ino = 500;

        tree.add_entry(ent, st);
    }

    tree.add_entry(dir_entry("bar/baz.txt", dir_entry::type_reg));

    // Check that there are only two entries foo and bar
    BOOST_CHECK_EQUAL(tree.subpath_dir("")->size(), 2);


    // Check foo directory

    dir_entry *foo = tree.get_entry("foo");
    BOOST_REQUIRE(foo);
    BOOST_CHECK_EQUAL(foo->attr().st_ino, 100);

    // Check bar directory

    dir_entry *bar = tree.get_entry("bar");
    BOOST_REQUIRE(bar);
    BOOST_CHECK_EQUAL(bar->attr().st_ino, 500);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(subdirectories)

BOOST_AUTO_TEST_CASE(base_directory) {
    archive_tree atree;
    dir_tree &tree = atree;

    tree.add_entry(dir_entry("foo.txt", dir_entry::type_reg));
    tree.add_entry(dir_entry("bar.x", dir_entry::type_reg));

    /// Check base directory

    auto base_dir = tree.subpath_dir("");
    BOOST_REQUIRE(base_dir);

    BOOST_CHECK_EQUAL(base_dir->size(), 2);
    BOOST_CHECK_EQUAL(tree.subpath().path(), "");

    // Check foo.txt

    auto foo_txt = base_dir->find("foo.txt");
    BOOST_REQUIRE(foo_txt != base_dir->end());
    BOOST_CHECK_EQUAL(foo_txt->second, tree.get_entry("foo.txt"));

    // Check bar.x

    auto bar_x = base_dir->find("bar.x");
    BOOST_REQUIRE(bar_x != base_dir->end());
    BOOST_CHECK_EQUAL(bar_x->second, tree.get_entry("bar.x"));
}

BOOST_AUTO_TEST_CASE(single_level) {
    archive_tree atree;
    dir_tree &tree = atree;

    tree.add_entry(dir_entry("foo.txt", dir_entry::type_reg));
    tree.add_entry(dir_entry("bar.x", dir_entry::type_reg));
    tree.add_entry(dir_entry("foo/baz.txt", dir_entry::type_reg));
    tree.add_entry(dir_entry("foo/bar/../baz/./../bar.txt", dir_entry::type_reg));
    tree.add_entry(dir_entry(".././foo", dir_entry::type_dir));

    BOOST_CHECK_EQUAL(tree.subpath().path(), "");


    /// Check foo subdirectory

    auto foo_dir = tree.subpath_dir("foo");
    BOOST_REQUIRE(foo_dir);
    BOOST_CHECK_EQUAL(foo_dir->size(), 2);

    // Check foo/bar.txt

    auto foo_bar = foo_dir->find("bar.txt");
    BOOST_REQUIRE(foo_bar != foo_dir->end());
    BOOST_CHECK_EQUAL(foo_bar->second, tree.get_entry("foo/bar.txt"));

    // Check foo/baz.txt

    auto foo_baz = foo_dir->find("baz.txt");
    BOOST_REQUIRE(foo_baz != foo_dir->end());
    BOOST_CHECK_EQUAL(foo_baz->second, tree.get_entry("foo/baz.txt"));

    // Check foo/

    dir_entry *foo = tree.get_entry("foo");
    BOOST_REQUIRE(foo);

    BOOST_CHECK_EQUAL(foo->subpath().path(), "foo");
    BOOST_CHECK_EQUAL(foo->type(), dir_entry::type_dir);


    /// Check get_entry from foo/ subdirectory

    tree.subpath("foo");
    BOOST_CHECK_EQUAL(tree.subpath().path(), "foo");

    BOOST_CHECK_EQUAL(foo_bar->second, tree.get_entry("bar.txt"));
    BOOST_CHECK_EQUAL(foo_baz->second, tree.get_entry("baz.txt"));
}

BOOST_AUTO_TEST_CASE(special_dirs) {
    archive_tree atree;
    dir_tree &tree = atree;

    tree.add_entry(dir_entry(".././foo", dir_entry::type_dir));
    tree.add_entry(dir_entry("/bar.txt", dir_entry::type_reg));

    /// Check .. directory

    auto parent = tree.subpath_dir("..");
    BOOST_REQUIRE(parent);

    // Check ../foo

    auto foo = parent->find("foo");
    BOOST_REQUIRE(foo != parent->end());
    BOOST_CHECK_EQUAL(foo->second, tree.get_entry("../foo"));


    /// Check / directory

    auto root = tree.subpath_dir("/");
    BOOST_REQUIRE(root);

    // Check /bar.txt

    auto bar = root->find("bar.txt");
    BOOST_REQUIRE(bar != root->end());
    BOOST_CHECK_EQUAL(bar->second, tree.get_entry("/bar.txt"));
}

BOOST_AUTO_TEST_CASE(multiple_levels) {
    archive_tree atree;
    dir_tree &tree = atree;

    tree.add_entry(dir_entry("foo/bar.txt", dir_entry::type_reg));
    tree.add_entry(dir_entry("foo/bar", dir_entry::type_dir));
    tree.add_entry(dir_entry("foo/bar/baz.txt", dir_entry::type_reg));
    tree.add_entry(dir_entry("foo", dir_entry::type_dir));

    BOOST_CHECK_EQUAL(tree.subpath().path(), "");


    /// Check base directory

    auto base = tree.subpath_dir("");
    BOOST_REQUIRE(base);
    BOOST_CHECK_EQUAL(base->size(), 1);

    // Check foo

    auto foo = base->find("foo");
    BOOST_REQUIRE(foo != base->end());
    BOOST_CHECK_EQUAL(foo->second, tree.get_entry("foo"));


    /// Check foo subdirectory

    auto foo_dir = tree.subpath_dir("foo");
    BOOST_REQUIRE(foo_dir);
    BOOST_CHECK_EQUAL(foo_dir->size(), 2);

    // Check foo/bar.txt file

    auto bar_txt = foo_dir->find("bar.txt");
    BOOST_REQUIRE(bar_txt != foo_dir->end());
    BOOST_CHECK_EQUAL(bar_txt->second, tree.get_entry("foo/bar.txt"));

    // Check foo/bar directory

    tree.subpath("foo");
    BOOST_CHECK_EQUAL(bar_txt->second, tree.get_entry("bar.txt"));

    auto bar = foo_dir->find("bar");
    BOOST_REQUIRE(bar != foo_dir->end());

    tree.subpath("");
    BOOST_CHECK_EQUAL(bar->second, tree.get_entry("foo/bar"));

    tree.subpath("foo");
    BOOST_CHECK_EQUAL(bar->second, tree.get_entry("bar"));


    /// Check foo/bar subdirectory

    auto bar_dir = tree.subpath_dir("foo/bar");
    BOOST_REQUIRE(bar_dir);
    BOOST_CHECK_EQUAL(bar_dir->size(), 1);

    // Check foo/bar/baz.txt file

    auto baz_txt = bar_dir->find("baz.txt");
    BOOST_REQUIRE(baz_txt != foo_dir->end());

    tree.subpath("");
    BOOST_CHECK_EQUAL(baz_txt->second, tree.get_entry("foo/bar/baz.txt"));

    tree.subpath("foo");
    BOOST_CHECK_EQUAL(baz_txt->second, tree.get_entry("bar/baz.txt"));

    tree.subpath("foo/bar");
    BOOST_CHECK_EQUAL(baz_txt->second, tree.get_entry("baz.txt"));
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE_END()
