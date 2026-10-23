AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -g
AM_LDFLAGS=-pthread
LDADD = -lstdc++ -lgtest -lgtest_main -lgmock -lgmock_main -lboost_iostreams
EXEEXT=.bin

TESTS = test_subprocess.bin test_guishell.bin test_textwidth.bin

check_PROGRAMS = test_subprocess test_guishell test_textwidth

test_subprocess_SOURCES = test_subprocess.cpp
test_subprocess_LDADD=../libsubprocess.la $(LDADD)

test_guishell_SOURCES = test_guishell.cpp
test_guishell_LDADD=../libguishell.la -lncursesw

test_textwidth_SOURCES = test_textwidth.cpp
test_textwidth_LDADD=../libguishell.la $(LDADD) -lncursesw
