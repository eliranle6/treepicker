#ifndef GUISHELL_H__
#define GUISHELL_H__

#include <streambuf>
#include <string>
#include <iostream>
#include <curses.h>
#include <unistd.h>
#include <ostream>

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
    SELECTED_DIRECTORY = 101,
    NON_SELECTED_DIRECTORY = 102,
    RED_ON_BLUE = 103,
    MAGENTA_ON_BLUE = 104
};

// The number of bytes from the start of s (n bytes of UTF-8, with no newline) that fit in
// nr_columns screen columns, without splitting a character
std::size_t nr_bytes_that_fit(const char *s, std::size_t n, int nr_columns);

// The curses color pair for a label of the given background color (e.g. "green", "red"), in
// white or black text, whichever reads better on it; -1 for an unknown color name. Needs curses.
int label_color_pair(const std::string& color);

// Whether label_color_pair() knows the color name
bool is_label_color(const std::string& color);

// The number of screen columns that s (UTF-8) takes
int display_width(const std::string& s);

// s with each control character (newline, tab, escape...) replaced by '?', like ls does, since
// printing them would move the cursor, or be taken as terminal commands
std::string printable(const std::string& s);

// A stream buffer that writes to the curses screen, when flushed (std::endl, a Color...) or full
class NCursesSink : public std::streambuf
{
public:
    NCursesSink(void) {
        setp(_data, _data + sizeof(_data));
    }

protected:
    int_type overflow(int_type c) override;
    int sync(void) override;

private:
    void write_buffer(void);
    std::streamsize write(const char* s, std::streamsize n);

    char _data[4096];
};



class GuiShell : public std::ostream
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
