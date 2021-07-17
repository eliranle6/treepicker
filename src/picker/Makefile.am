AM_CXXFLAGS = $(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -pthread
SUBDIRS = tests
EXEXT=bin
LDADD= $(top_builddir)/src/utils/libsubprocess.la $(top_builddir)/src/tree/libtree.la $(top_builddir)/src/treeprinter/libtreeprinter.la -lboost_system -lboost_filesystem -lboost_iostreams

bin_PROGRAMS = picker
lib_LTLIBRARIES = libpicker.la

libpicker_la_LIBADD = $(LIBS) -lncursesw $(top_builddir)/src/tree/libtree.la $(top_builddir)/src/treeprinter/libtreeprinter.la
libpicker_la_SOURCES = picker.cpp keyboardreactor.cpp tree_selector.cpp tree_keyboard_selector.cpp

picker_LDADD = $(LDADD) libpicker.la -lncursesw $(top_builddir)/src/utils/libguishell.la
picker_SOURCES = main.cpp
