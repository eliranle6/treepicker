#ifndef GUISHELL_H__
#define GUISHELL_H__

#include <streambuf>
#include <string>
#include <vector>
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
    GREY,
    GREEN,
    CYAN,
    // Of 256 colors, 208; with fewer, yellow
    ORANGE,
    NR_STYLES
};

// The style of text in the named color (red, green, yellow, blue, magenta, cyan, grey, orange), or
// DEFAULT for another name
Style text_style(const std::string& color);

// A part of a text in styles (see parse_styled())
struct StyledPart {
    std::string text;
    Style style;
    bool is_bold;

    bool operator==(const StyledPart& other) const {
        return text == other.text and style == other.style and is_bold == other.is_bold;
    }
};

// A text in styled parts: plain, until STYLE_MARK, a style and STYLE_MARK again, after which it's
// in that style, until the next such mark. A style is words, separated by commas or spaces: a
// color's name (see text_style()) and "bold"; an empty one is plain again. Empty parts are dropped,
// and a mark without its style's end is dropped with the rest of the text.
constexpr char STYLE_MARK = '\x02';
std::vector<StyledPart> parse_styled(const std::string& s);
// The text of s without its styles' marks
std::string unstyled(const std::string& s);

// The curses color pair of a style, for attron()
int color_pair(Style style);

// The number of bytes from the start of s (n bytes of UTF-8, with no newline) that fit in
// nr_columns screen columns, without splitting a character
std::size_t nr_bytes_that_fit(const char *s, std::size_t n, int nr_columns);

// The curses color pair for a label of the given background color (e.g. "green", "red"), in
// white or black text, whichever reads better on it; -1 for an unknown color name. Needs curses.
int label_color_pair(const std::string& color);

// Whether the terminal shows images (picker/image.h): with 256 colors, and room for their pairs
bool can_show_images(void);

// The curses color pair of an image's character: foreground and background colors of 256 (-1 for
// the terminal's default). Pairs are made as needed; reset_image_color_pairs() starts them over,
// e.g. before drawing an image, if there may not be room for its own. Needs curses.
short image_color_pair(short foreground, short background);
void reset_image_color_pairs(void);
// How many pairs image_color_pair() can make before reset_image_color_pairs() is needed
int nr_image_color_pairs_left(void);

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

  // Cut the lines written from now on at nr_columns from the screen's left edge, e.g. to leave
  // room beside them; 0: at the screen's edge, less its last column (the default)
  void set_line_width(int nr_columns);

  // Start the lines written from now on (after the first, which starts where the cursor is) at
  // nr_columns from the screen's left edge, e.g. for a tree in a column on the right; 0: at it
  void set_line_origin(int nr_columns);

  // The terminal's file descriptor, which keys are read from
  int terminal_fd(void) const { return _terminal_fd; }

private:
  int _terminal_fd;

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
