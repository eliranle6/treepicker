AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
AM_LDFLAGS = -pthread
LDADD = -lstdc++ -lgtest -lgtest_main -lgmock -lgmock_main
EXEEXT=.bin

TESTS = test_tree.bin \
		test_search.bin

check_PROGRAMS = test_tree \
				 test_search

test_tree_SOURCES = test_tree.cpp
test_search_SOURCES = test_search.cpp
