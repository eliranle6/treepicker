AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests

lib_LTLIBRARIES = libsubprocess.la libguishell.la

libsubprocess_la_SOURCES = subprocess.cpp
libsubprocess_la_LIBADD = -lboost_iostreams

libguishell_la_SOURCES = guishell.cpp
libguishell_la_LIBADD = -lncurses -lboost_filesystem
