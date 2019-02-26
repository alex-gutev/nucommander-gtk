check_PROGRAMS = nucommander-test

nucommander_test_SOURCES = pathname_test.cpp \
	../src/paths/pathname.h \
	../src/paths/pathname.cpp

nucommander_test_CPPFLAGS = -I$(top_srcdir)/src $(BOOST_CPPFLAGS)
nucommander_test_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIB)

TESTS = nucommander-test
