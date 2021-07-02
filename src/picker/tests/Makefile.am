AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -g -O0
AM_LDFLAGS = -pthread
LOCALLIBS = $(top_builddir)/src/tree/libtree.la
LDADD = $(LOCALLIBS) -lstdc++ -lgtest -lgtest_main -lgmock -lgmock_main $(top_builddir)/src/utils/libguishell.la
EXEEXT=.bin

TESTS = test_tree_selector.bin

check_PROGRAMS = test_tree_selector

test_tree_selector_SOURCES = test_tree_selector.cpp
test_tree_selector_LDADD = $(LDADD) $(top_builddir)/src/treeprinter/libpaginator.la
