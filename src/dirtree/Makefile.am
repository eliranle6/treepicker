AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests
LIBADD = -lboost_filesystem

lib_LIBRARIES = libdirtree.a

libdirtree_a_SOURCES = dirtree.cpp
libdirtree_a_LIBADD = $(LDADD) $(top_builddir)/src/tree/libtree.a $(top_builddir)/src/utils/libsubprocess.a
