AM_CXXFLAGS = $(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -pthread
SUBDIRS = tests
EXEXT=bin
LDADD= $(top_builddir)/src/utils/libguishell.la $(top_builddir)/src/utils/libsubprocess.la $(top_builddir)/src/treeprinter/libtreeprinter.la -lboost_system -lboost_filesystem -lboost_iostreams -lncurses

bin_PROGRAMS = picker
lib_LTLIBRARIES = libpicker.la

picker_LDADD = $(LDADD)
picker_SOURCES = main.cpp libpicker.la

#libpicker_a_LIBADD= $(LIBS)
libpicker_la_SOURCES = picker.cpp
