AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
AM_LDFLAGS = -pthread
LDADD = -lstdc++ -lgtest -lgtest_main -lgmock -lgmock_main
EXEEXT=.bin

TESTS = test_tree.bin \
		test_treeprinter.bin \
		test_pre_print_tree_analysis.bin \
		test_search.bin

check_PROGRAMS = test_tree \
				 test_treeprinter \
				 test_pre_print_tree_analysis \
				 test_search

test_treeprinter_SOURCES = test_treeprinter.cpp
test_pre_print_tree_analysis_SOURCES = test_pre_print_tree_analysis.cpp
test_tree_SOURCES = test_tree.cpp
test_search_SOURCES = test_search.cpp
