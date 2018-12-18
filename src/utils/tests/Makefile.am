AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -g
AM_LDFLAGS=-pthread
LDADD = -lstdc++ -lgtest -lgtest_main -lgmock -lgmock_main -lboost_iostreams
EXEEXT=.bin

TESTS = test_subprocess.bin

check_PROGRAMS = test_subprocess

test_subprocess_SOURCES = test_subprocess.cpp
test_subprocess_LDADD=../libsubprocess.la $(LDADD)
