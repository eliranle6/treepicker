AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests

lib_LIBRARIES = libdirtree.a

libdirtree_a_SOURCES = dirtree.cpp
libdirtree_a_LIBADD=$(top_builddir)/src/tree/libtree.a
