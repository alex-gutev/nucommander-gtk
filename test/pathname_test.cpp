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

#include <boost/test/unit_test.hpp>

#include <string>
#include <set>
#include <iostream>

#include "paths/pathname.h"

using namespace nuc::paths;

BOOST_AUTO_TEST_SUITE(pathname_tests);

BOOST_AUTO_TEST_CASE(constructor1) {
    string path_str("/foo/bar/baz");
    pathname path(path_str);

    BOOST_CHECK_EQUAL(path_str, path.path());
}
BOOST_AUTO_TEST_CASE(constructor2) {
    string path_str("/foo/bar/baz");
    pathname path(path_str, true);

    BOOST_CHECK_EQUAL((path_str + "/"), path.path());
}
BOOST_AUTO_TEST_CASE(constructor3) {
    string path_str("/foo/bar/baz/");
    pathname path(path_str, false);

    BOOST_CHECK_EQUAL("/foo/bar/baz", path.path());
}

BOOST_AUTO_TEST_CASE(append1) {
    pathname path1("/foo/bar");
    pathname path2 = path1.append("baz");

    BOOST_CHECK_EQUAL(path2.path(), "/foo/bar/baz");
}
BOOST_AUTO_TEST_CASE(append2) {
    pathname path1("/foo");
    pathname path2 = path1.append("bar/baz");

    BOOST_CHECK_EQUAL(path2.path(), "/foo/bar/baz");
}
BOOST_AUTO_TEST_CASE(append3) {
    pathname path1("/foo/");
    pathname path2 = path1.append("bar/baz");

    BOOST_CHECK_EQUAL(path2.path(), "/foo/bar/baz");
}

BOOST_AUTO_TEST_CASE(remove_last1) {
    pathname path = pathname("/foo/bar/baz").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "/foo/bar");
}
BOOST_AUTO_TEST_CASE(remove_last2) {
    pathname path = pathname("/foo/bar/baz/").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "/foo/bar/");
}
BOOST_AUTO_TEST_CASE(remove_last3) {
    pathname path = pathname("/foo/").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "/");
}
BOOST_AUTO_TEST_CASE(remove_last4) {
    pathname path = pathname("/").remove_last_component();

    BOOST_CHECK_EQUAL(path.path(), "/");
}

BOOST_AUTO_TEST_CASE(merge1) {
    pathname path1 = pathname("/foo/bar");
    pathname path2 = pathname("baz.txt");

    BOOST_CHECK_EQUAL(path1.merge(path2).path(), "/foo/baz.txt");
}
BOOST_AUTO_TEST_CASE(merge2) {
    pathname path1 = pathname("/foo/bar/");
    pathname path2 = pathname("baz.txt");

    BOOST_CHECK_EQUAL(path1.merge(path2).path(), "/foo/bar/baz.txt");
}
BOOST_AUTO_TEST_CASE(merge3) {
    pathname path1 = pathname("/foo/bar/");
    pathname path2 = pathname("/baz/file.txt");

    BOOST_CHECK_EQUAL(path1.merge(path2).path(), "/baz/file.txt");
}
BOOST_AUTO_TEST_CASE(merge4) {
    pathname path1 = pathname("/foo/bar/");
    pathname path2 = pathname("~/baz/file.txt");

    BOOST_CHECK_EQUAL(path1.merge(path2).path(), "~/baz/file.txt");
}

BOOST_AUTO_TEST_CASE(canonicalize1) {
    pathname path = pathname("a/relative/../path/./").canonicalize();

    BOOST_CHECK_EQUAL(path.path(), "a/path");
}
BOOST_AUTO_TEST_CASE(canonicalize2) {
    pathname path = pathname("a/relative/../path/./").canonicalize(true);

    BOOST_CHECK_EQUAL(path.path(), "a/path/");
}
BOOST_AUTO_TEST_CASE(canonicalize3) {
    pathname path = pathname("a/relative/../../../path/./dir").canonicalize();

    BOOST_CHECK_EQUAL(path.path(), "../path/dir");
}
BOOST_AUTO_TEST_CASE(canonicalize4) {
    pathname path = pathname("a/relative/../../../path/./dir").canonicalize(true);

    BOOST_CHECK_EQUAL(path.path(), "../path/dir/");
}
BOOST_AUTO_TEST_CASE(canonicalize5) {
    pathname path = pathname(".././../a///bad/path/").canonicalize();

    BOOST_CHECK_EQUAL(path.path(), "../../a/bad/path");
}
BOOST_AUTO_TEST_CASE(canonicalize6) {
    pathname path = pathname(".././../a///bad/path/").canonicalize(true);

    BOOST_CHECK_EQUAL(path.path(), "../../a/bad/path/");
}
BOOST_AUTO_TEST_CASE(canonicalize7) {
    pathname path = pathname("/../dir/file").canonicalize();

    BOOST_CHECK_EQUAL(path.path(), "dir/file");
}
BOOST_AUTO_TEST_CASE(canonicalize8) {
    BOOST_CHECK_EQUAL(pathname("/").canonicalize().path(), "/");
}

BOOST_AUTO_TEST_CASE(basename1) {
    pathname path = pathname("/foo/bar/baz.txt");

    BOOST_CHECK_EQUAL(path.basename(), "baz.txt");
    BOOST_CHECK_EQUAL(path.extension(), "txt");
}
BOOST_AUTO_TEST_CASE(basename2) {
    pathname path = pathname("/foo/bar/baz");

    BOOST_CHECK_EQUAL(path.basename(), "baz");
    BOOST_CHECK_EQUAL(path.extension(), "");
}
BOOST_AUTO_TEST_CASE(basename3) {
    pathname path = pathname("/foo/bar/baz/");

    BOOST_CHECK_EQUAL(path.basename(), "baz");
    BOOST_CHECK_EQUAL(path.extension(), "");
}
BOOST_AUTO_TEST_CASE(basename4) {
    pathname path = pathname("foo");

    BOOST_CHECK_EQUAL(path.basename(), "foo");
}
BOOST_AUTO_TEST_CASE(basename5) {
    pathname path = pathname("/");

    BOOST_CHECK_EQUAL(path.basename(), "");
}


BOOST_AUTO_TEST_CASE(is_root1) {
    pathname path = pathname("/foo");

    BOOST_CHECK(!path.is_root());
}
BOOST_AUTO_TEST_CASE(is_root2) {
    pathname path = pathname("/");

    BOOST_CHECK(path.is_root());
}
BOOST_AUTO_TEST_CASE(is_root3) {
    pathname path = pathname("foo");

    BOOST_CHECK(!path.is_root());
}
BOOST_AUTO_TEST_CASE(is_root4) {
    pathname path = pathname("");

    BOOST_CHECK(!path.is_root());
}

BOOST_AUTO_TEST_CASE(is_relative1) {
    pathname path = pathname("foo/bar");

    BOOST_CHECK(path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative2) {
    pathname path = pathname("foo");

    BOOST_CHECK(path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative3) {
    pathname path = pathname("");

    BOOST_CHECK(path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative4) {
    pathname path = pathname("/foo/bar");

    BOOST_CHECK(!path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative5) {
    pathname path = pathname("/");

    BOOST_CHECK(!path.is_relative());
}
BOOST_AUTO_TEST_CASE(is_relative6) {
    pathname path = pathname("~user");

    BOOST_CHECK(!path.is_relative());
}

BOOST_AUTO_TEST_CASE(is_subpath1) {
    pathname parent = pathname("/foo/bar");
    pathname child = pathname("/foo/bar/baz/file.txt");

    BOOST_CHECK(child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath2) {
    pathname parent = pathname("/foo/bar");
    pathname child = pathname("/foo/baz/file.txt");

    BOOST_CHECK(!child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath3) {
    pathname parent = pathname("/foo/bar/");
    pathname child = pathname("/foo/bar");

    BOOST_CHECK(!child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath4) {
    pathname parent = pathname("/foo/bar");
    pathname child = pathname("/foo/bar");

    BOOST_CHECK(!child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath5) {
    pathname parent = pathname("/foo/bar");
    pathname child = pathname("/foo/bar.txt");

    BOOST_CHECK(!child.is_subpath(parent));
}
BOOST_AUTO_TEST_CASE(is_subpath6) {
    pathname parent = pathname("");
    pathname child = pathname("foo");

    BOOST_CHECK(child.is_subpath(parent));
}


BOOST_AUTO_TEST_CASE(is_child_of1) {
    pathname parent = pathname("/foo/bar");
    pathname child = pathname("/foo/bar/file.txt");

    BOOST_CHECK(child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of2) {
    pathname parent = pathname("/foo/bar");
    pathname child = pathname("/foo/bar/baz/file.txt");

    BOOST_CHECK(!child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of3) {
    pathname parent = pathname("/foo/bar");
    pathname child = pathname("/foo/baz/file.txt");

    BOOST_CHECK(!child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of4) {
    pathname parent = pathname("/foo/bar/");
    pathname child = pathname("/foo/bar");

    BOOST_CHECK(!child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of5) {
    pathname parent = pathname("/foo/bar");
    pathname child = pathname("/foo/bar");

    BOOST_CHECK(!child.is_child_of(parent));
}
BOOST_AUTO_TEST_CASE(is_child_of6) {
    pathname parent = pathname("");
    pathname child = pathname("foo");

    BOOST_CHECK(child.is_child_of(parent));
}

BOOST_AUTO_TEST_CASE(has_dirs1) {
    pathname path = pathname("/foo/bar");

    BOOST_CHECK(path.has_dirs());
}
BOOST_AUTO_TEST_CASE(has_dirs2) {
    pathname path = pathname("foo.txt");

    BOOST_CHECK(!path.has_dirs());
}
BOOST_AUTO_TEST_CASE(has_dirs3) {
    pathname path = pathname("/");

    BOOST_CHECK(path.has_dirs());
}
BOOST_AUTO_TEST_CASE(has_dirs4) {
    pathname path = pathname("");

    BOOST_CHECK(!path.has_dirs());
}

BOOST_AUTO_TEST_CASE(subpath_offset) {
    std::set<pathname> paths;

    paths.insert("/foo/bar");
    paths.insert("/foo/baz/");
    paths.insert("/foo/dir/");

    BOOST_CHECK_EQUAL(pathname::subpath_offset(paths, "/foo/bar"), 5);
    BOOST_CHECK_EQUAL(pathname::subpath_offset(paths, "/foo/bar.txt"), string::npos);
    BOOST_CHECK_EQUAL(pathname::subpath_offset(paths, "/foo/bar/file.txt"), string::npos);

    BOOST_CHECK_EQUAL(pathname::subpath_offset(paths, "/foo/baz/"), 5);
    BOOST_CHECK_EQUAL(pathname::subpath_offset(paths, "/foo/baz/file.txt"), 5);
    BOOST_CHECK_EQUAL(pathname::subpath_offset(paths, "/foo/baz/bar/file.txt"), 5);
    BOOST_CHECK_EQUAL(pathname::subpath_offset(paths, "/foo/baz.txt"), string::npos);

    BOOST_CHECK_EQUAL(pathname::subpath_offset(paths, "something else"), string::npos);
    BOOST_CHECK_EQUAL(pathname::subpath_offset(paths, "/foo/zzz"), string::npos);
}

BOOST_AUTO_TEST_SUITE_END();
