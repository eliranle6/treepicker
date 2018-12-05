AM_CXXFLAGS = $(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -pthread
SUBDIRS = tests
EXEXT=bin
LDADD= $(top_builddir)/src/guishell/libguishell.a $(top_builddir)/src/utils/libsubprocess.a -lboost_system -lboost_filesystem -lboost_iostreams -lncurses

bin_PROGRAMS = picker
lib_LIBRARIES = libpicker.a

picker_LDADD= $(LDADD)
picker_SOURCES = main.cpp picker.cpp

#libpicker_a_LIBADD= $(LIBS)
libpicker_a_SOURCES = picker.cpp
