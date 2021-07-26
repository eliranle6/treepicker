AM_CXXFLAGS = $(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests

lib_LTLIBRARIES = libtreeprinter.la libpaginator.la

libtreeprinter_la_SOURCES = \
	treeprinter.cpp \
	paginator.cpp \
	treewindowfitter.cpp \
	printnode.cpp

libtreeprinter_la_LIBADD = -lncursesw $(top_builddir)/src/utils/libguishell.la

libpaginator_la_SOURCES = paginator.cpp
libpaginator_la_LIBADD = $(top_builddir)/src/tree/libtree.la
