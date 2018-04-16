AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror
AM_LDFLAGS=$(LDFLAGS) -lstdc++

bin_PROGRAMS = test_tree \
               test_treeprinter

test_tree_SOURCES = test_tree.cpp
test_treeprinter_SOURCES = test_treeprinter.cpp

noinst_LIBRARIES = libtreelib.a
libtreelib_a_SOURCES = treelib.h
