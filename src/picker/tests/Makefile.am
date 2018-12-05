AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
AM_LDFLAGS = -pthread
LOCALLIBS = $(top_builddir)/src/tree/libtree.a
LDADD = $(LOCALLIBS) -lstdc++ -lgtest -lgtest_main -lgmock -lgmock_main
EXEEXT=.bin

TESTS = test_tree_selector.bin

check_PROGRAMS = test_tree_selector

test_tree_selector_SOURCES = test_tree_selector.cpp
test_tree_selector_LDADD = $(LDADD)
