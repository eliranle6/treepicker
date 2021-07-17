AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -g
AM_LDFLAGS = -pthread
LDADD = -lstdc++ -lgtest -lgtest_main -lgmock -lgmock_main $(top_builddir)/src/tree/libtree.la $(top_builddir)/src/utils/libguishell.la $(top_builddir)/src/treeprinter/libtreeprinter.la
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
test_treeprinter_LDADD = -lncursesw $(LDADD)

test_pre_print_tree_analysis_SOURCES = test_pre_print_tree_analysis.cpp
test_pre_print_tree_analysis_LDADD = $(LDADD)

test_treewindowfitter_SOURCES = test_treewindowfitter.cpp
test_treewindowfitter_LDADD = -lncursesw $(LDADD)

test_paginator_SOURCES = test_paginator.cpp
test_paginator_LDADD = -lstdc++ -lgtest -lgtest_main -lgmock -lgmock_main $(top_builddir)/src/treeprinter/libtreeprinter.la $(top_builddir)/src/tree/libtree.la
