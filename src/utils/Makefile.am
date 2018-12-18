AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
SUBDIRS = tests

lib_LTLIBRARIES = libsubprocess.la

libsubprocess_la_SOURCES = subprocess.cpp
libsubprocess_la_LIBADD = -lboost_iostreams
