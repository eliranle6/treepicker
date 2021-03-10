#ifndef GUISHELL_H__
#define GUISHELL_H__

#include <streambuf>
#include <iostream>
#include <curses.h>
#include <unistd.h>
#include <boost/iostreams/stream.hpp>

namespace guishell {

enum Colors {
    DEFAULT = 39,
    BLACK = 30,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    LIGHT_GRAY = 37,
    DARK_GRAY = 90,
    LIGHT_RED = 91,
    LIGHT_GREEN = 92,
    LIGHT_YELLOW = 93,
    LIGHT_BLUE = 94,
    LIGHT_MAGENTA = 95,
    LIGHT_CYAN = 96,
    WHITE = 97,
    WHITE_ON_BLUE = 98,
    BLACK_ON_BLUE = 99,
    BLACK_ON_WHITE = 100,
    DIRECTORY = 12,
    SELECTED_DIRECTORY = 100,
    NON_SELECTED_DIRECTORY = 101
};

class NCursesSink : public boost::iostreams::sink
{
public:
    std::streamsize write(const char* s, std::streamsize n);
};


using GuiShellStream = boost::iostreams::stream<NCursesSink>;

class GuiShell : public GuiShellStream
{
public:
  static bool is_initialized;

  static GuiShell& get_instance(void);

  ~GuiShell(void);

  void refresh(void);

  void clear(void);

private:
  GuiShell(NCursesSink &buff);
};

class Color
{
public:
    Color(int code);
    friend std::ostream& operator<< (std::ostream &out, const Color &color);

private:
    int _code;
};

class Bold
{
public:
    friend std::ostream& operator<< (std::ostream &out, const Bold &color);
};

class Unbold
{
public:
    friend std::ostream& operator<< (std::ostream &out, const Unbold &color);
};

}

#endif
