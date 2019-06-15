AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
AM_LDFLAGS = -pthread
LDADD = -lstdc++ -lgtest -lgtest_main -lgmock -lgmock_main
EXEEXT=.bin

TESTS = test_treeprinter.bin \
		test_pre_print_tree_analysis.bin \
		test_treewindowfitter.bin \
		test_paginator.bin

check_PROGRAMS = test_treeprinter \
				 test_pre_print_tree_analysis \
			     test_treewindowfitter \
				 test_paginator

test_treeprinter_SOURCES = test_treeprinter.cpp
test_treeprinter_LDADD = -lncurses $(LDADD) $(top_builddir)/src/treeprinter/libtreeprinter.la

test_pre_print_tree_analysis_SOURCES = test_pre_print_tree_analysis.cpp

test_treewindowfitter_SOURCES = test_treewindowfitter.cpp
test_treewindowfitter_LDADD = -lncurses $(LDADD)

test_paginator_SOURCES = test_paginator.cpp
test_paginator_LDADD = $(LDADD) $(top_builddir)/src/treeprinter/libtreeprinter.la
