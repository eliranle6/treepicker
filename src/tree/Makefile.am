AM_CXXFLAGS = $(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests

lib_LIBRARIES = libtree.a

libtree_a_LIBADD=$(top_builddir)/src/utils/libsubprocess.a
libtree_a_SOURCES = tree.cpp
