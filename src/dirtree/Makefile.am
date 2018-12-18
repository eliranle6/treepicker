AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests
LIBADD = -lboost_filesystem

lib_LTLIBRARIES = libdirtree.la

libdirtree_la_SOURCES = dirtree.cpp
libdirtree_la_LIBADD = $(LDADD) $(top_builddir)/src/tree/libtree.la $(top_builddir)/src/utils/libsubprocess.la
