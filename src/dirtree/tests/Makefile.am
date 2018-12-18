AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/
AM_LDFLAGS = -pthread
LDADD = -lstdc++ -lboost_iostreams -lgtest -lgtest_main -lgmock -lgmock_main -lboost_system -lboost_filesystem
EXEEXT=.bin

TESTS = test_dirtree.bin
check_PROGRAMS = test_dirtree

test_dirtree_SOURCES = test_dirtree.cpp
test_dirtree_LDADD=../libdirtree.la ../../utils/libsubprocess.la $(LDADD)
