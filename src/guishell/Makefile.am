AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests

LIBADD = -lncurses -lboost_filesystem

lib_LIBRARIES = libguishell.a

libguishell_a_SOURCES = guishell.cpp
#libguishell_a_LIBADD = $(LIBADD)
