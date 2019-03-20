check_PROGRAMS = test-pathname test-directory-tree

TESTS = test-pathname test-directory-tree


# Pathname Tests

test_pathname_SOURCES = pathname_test.cpp

test_pathname_CPPFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS)
test_pathname_LDFLAGS = $(BOOST_LDFLAGS)
test_pathname_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIB) \
	../src/paths/nucommander-pathname.$(OBJEXT)


# Directory Tree Tests

test_directory_tree_SOURCES = dir_tree_test.cpp
test_directory_tree_CPPFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS) $(GLIBMM_CFLAGS)
test_directory_tree_LDFLAGS = $(BOOST_LDFLAGS)
test_directory_tree_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIB) \
	$(GLIBMM_LIBS) \
	../src/paths/nucommander-pathname.$(OBJEXT) \
	../src/directory/nucommander-dir_entry.$(OBJEXT) \
	../src/directory/nucommander-dir_tree.$(OBJEXT) \
	../src/directory/nucommander-archive_tree.$(OBJEXT)
