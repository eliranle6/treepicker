AM_CXXFLAGS=$(CXXFLAGS) -std=c++1z -Wall -Werror -I$(top_srcdir)/src/ -g

LDADD=-lncurses
EXEEXT=.bin

TESTS = test_guishell.bin
check_PROGRAMS = test_guishell

test_guishell_SOURCES = test_guishell.cpp
test_guishell_LDADD=../libguishell.a $(LDADD)

