#ifndef GUISHELL_H__
#define GUISHELL_H__

#include <streambuf>
#include <string>
#include <iostream>
#include <curses.h>
#include <unistd.h>
#include <ostream>

namespace guishell {

// How text is drawn. Each style has its curses colors, and its ANSI escape code for when curses
// isn't running (see guishell.cpp's STYLES); its curses color pair is its own, not a color code.
enum Style {
    DEFAULT,
    RED,
    MAGENTA,
    YELLOW,
    BLUE,
    DIRECTORY,
    SELECTED_DIRECTORY,
    WHITE_ON_BLUE,
    BLACK_ON_BLUE,
    BLACK_ON_WHITE,
    RED_ON_BLUE,
    MAGENTA_ON_BLUE,
    NR_STYLES
};

// The curses color pair of a style, for attron()
int color_pair(Style style);

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

// Written to a stream, switches it to a style
class Color
{
public:
    Color(Style style);
    friend std::ostream& operator<< (std::ostream &out, const Color &color);

private:
    Style _style;
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
