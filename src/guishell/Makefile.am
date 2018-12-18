AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests

LIBADD = -lncurses -lboost_filesystem

lib_LTLIBRARIES = libguishell.la

libguishell_la_SOURCES = guishell.cpp