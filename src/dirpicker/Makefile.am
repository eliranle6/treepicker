AM_CXXFLAGS = $(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -pthread
EXEXT=bin
LDADD= $(top_builddir)/src/dirtree/libdirtree.la $(top_builddir)/src/picker/libpicker.la $(top_builddir)/src/utils/libsubprocess.la $(top_builddir)/src/utils/libguishell.la $(top_builddir)/src/treeprinter/libtreeprinter.la -lboost_system -lboost_filesystem -lboost_iostreams -lncursesw

bin_PROGRAMS = dirpicker

dirpicker_LDADD= $(LDADD)
dirpicker_SOURCES = dirpicker.cpp main.cpp

