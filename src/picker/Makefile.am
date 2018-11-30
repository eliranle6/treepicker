AM_CXXFLAGS = $(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -pthread
SUBDIRS = tests
EXEXT=bin
LDADD=-lboost_system -lncurses $(top_builddir)/src/guishell/libguishell.a

bin_PROGRAMS = picker

#picker_LDADD=$(top_builddir)/src/utils/libsubprocess.a
picker_SOURCES = main.cpp menu.cpp
