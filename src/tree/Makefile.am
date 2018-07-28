AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I..
AM_LDFLAGS=$(LDFLAGS) -lstdc++
EXEEXT=.bin

## Tests
TESTS = test_tree.bin \
		test_treeprinter.bin \
		test_pre_print_tree_analysis.bin

check_PROGRAMS = test_tree \
				 test_treeprinter \
				 test_pre_print_tree_analysis

test_treeprinter_SOURCES = test_treeprinter.cpp
test_pre_print_tree_analysis_SOURCES = test_pre_print_tree_analysis.cpp
test_tree_SOURCES = test_tree.cpp
