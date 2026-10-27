#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <cwchar>
#include <limits>
#include <streambuf>
#include <iostream>
#include <curses.h>
#include <unistd.h>

#include "guishell.h"

namespace guishell {

// Walk s (n bytes of UTF-8) while it fits in max_nr_columns screen columns; return the number of
// bytes walked, and set nr_columns_used to their width
static std::size_t measure(const char *s, std::size_t n, int max_nr_columns,
                           int& nr_columns_used) {
    std::mbstate_t state = std::mbstate_t();
    std::size_t nr_bytes = 0;
    nr_columns_used = 0;
    while (nr_bytes < n) {
        wchar_t wc;
        std::size_t char_size = std::mbrtowc(&wc, s + nr_bytes, n - nr_bytes, &state);
        int char_width;
        if (char_size == static_cast<std::size_t>(-1) or char_size == static_cast<std::size_t>(-2)) {
            // Not valid (or not complete) UTF-8: count the byte as one column
            state = std::mbstate_t();
            char_size = 1;
            char_width = 1;
        } else {
            if (char_size == 0) {
                char_size = 1;  // A null character
            }
            // Non-printable characters take no columns
            char_width = std::max(0, wcwidth(wc));
        }
        if (nr_columns_used + char_width > max_nr_columns) {
            break;
        }
        nr_columns_used += char_width;
        nr_bytes += char_size;
    }
    return nr_bytes;
}

std::size_t nr_bytes_that_fit(const char *s, std::size_t n, int nr_columns) {
    int nr_columns_used;
    return measure(s, n, nr_columns, nr_columns_used);
}

int display_width(const std::string& s) {
    int nr_columns_used;
    measure(s.data(), s.size(), std::numeric_limits<int>::max(), nr_columns_used);
    return nr_columns_used;
}

// Whether text reached past the bottom of the screen since it was last cleared. The sink is copied
// into the stream, so this lives outside it.
static bool is_screen_full = false;

namespace {
struct LabelColor {
    const char* name;
    short background;
    short foreground;
};
const LabelColor LABEL_COLORS[] = {
    {"red", COLOR_RED, COLOR_WHITE},
    {"green", COLOR_GREEN, COLOR_BLACK},
    {"yellow", COLOR_YELLOW, COLOR_BLACK},
    {"blue", COLOR_BLUE, COLOR_WHITE},
    {"magenta", COLOR_MAGENTA, COLOR_WHITE},
    {"cyan", COLOR_CYAN, COLOR_BLACK},
};

struct StyleColors {
    Style style;
    short foreground;
    short background;
    const char* ansi_code;
};
// -1: the terminal's default. Color 12 is bright blue (of 16 colors), which is ANSI's 94.
const StyleColors STYLES[] = {
    {DEFAULT, -1, -1, "39;49"},
    {RED, COLOR_RED, -1, "31"},
    {MAGENTA, COLOR_MAGENTA, -1, "35"},
    {YELLOW, COLOR_YELLOW, -1, "33"},
    {BLUE, COLOR_BLUE, -1, "34"},
    {DIRECTORY, 12, -1, "94"},
    {SELECTED_DIRECTORY, 12, COLOR_BLUE, "94;44"},
    {WHITE_ON_BLUE, COLOR_WHITE, COLOR_BLUE, "37;44"},
    {BLACK_ON_BLUE, COLOR_BLACK, COLOR_BLUE, "30;44"},
    {BLACK_ON_WHITE, COLOR_BLACK, COLOR_WHITE, "30;47"},
    {RED_ON_BLUE, COLOR_RED, COLOR_BLUE, "31;44"},
    {MAGENTA_ON_BLUE, COLOR_MAGENTA, COLOR_BLUE, "35;44"},
    // Color 8 is bright black, which is ANSI's 90
    {GREY, 8, -1, "90"},
};
static_assert(sizeof(STYLES) / sizeof(STYLES[0]) == NR_STYLES, "a style without colors");

// Pair 0 is curses' own (the default colors, unchangeable): styles' pairs start at 1
const short FIRST_STYLE_COLOR_PAIR = 1;
// Past the styles' pairs
const short FIRST_LABEL_COLOR_PAIR = FIRST_STYLE_COLOR_PAIR + NR_STYLES;
}

int color_pair(Style style) {
    return FIRST_STYLE_COLOR_PAIR + style;
}

bool is_label_color(const std::string& color) {
    for (auto& label_color : LABEL_COLORS) {
        if (color == label_color.name) {
            return true;
        }
    }
    return false;
}

int label_color_pair(const std::string& color) {
    for (std::size_t i = 0; i < sizeof(LABEL_COLORS) / sizeof(LABEL_COLORS[0]); ++i) {
        if (color == LABEL_COLORS[i].name) {
            const short pair = FIRST_LABEL_COLOR_PAIR + i;
            init_pair(pair, LABEL_COLORS[i].foreground, LABEL_COLORS[i].background);
            return pair;
        }
    }
    return -1;
}

std::string printable(const std::string& s) {
    std::string result = s;
    for (auto& c : result) {
        const auto byte = static_cast<unsigned char>(c);
        if (byte < ' ' or byte == 0x7f) {
            c = '?';
        }
    }
    return result;
}

std::streamsize NCursesSink::write(const char *s, std::streamsize n) {
    // Print line by line, cutting each line at the right edge of the screen, since a wrapped line
    // would push the rest of the tree down. The last column is left empty, since writing to it
    // moves the cursor to the next line. Not printw: text may contain '%'.
    // Once the screen is full, drop the rest: on the bottom row a newline can't move the cursor
    // down, so the next line would be appended to the bottom row.
    const char *end = s + n;
    while (s < end and not is_screen_full) {
        const char *newline = std::find(s, end, '\n');
        const int nr_columns_left = COLS - 1 - getcurx(stdscr);
        if (nr_columns_left > 0) {
            addnstr(s, nr_bytes_that_fit(s, newline - s, nr_columns_left));
        }
        if (newline != end) {
            if (getcury(stdscr) >= LINES - 1) {
                is_screen_full = true;
            } else {
                addch('\n');
            }
            s = newline + 1;
        } else {
            s = end;
        }
    }
    return n;
}

NCursesSink::int_type NCursesSink::overflow(int_type c) {
    write_buffer();
    if (not traits_type::eq_int_type(c, traits_type::eof())) {
        *pptr() = traits_type::to_char_type(c);
        pbump(1);
    }
    return traits_type::not_eof(c);
}

int NCursesSink::sync(void) {
    write_buffer();
    return 0;
}

void NCursesSink::write_buffer(void) {
    write(pbase(), pptr() - pbase());
    setp(_data, _data + sizeof(_data));
}

GuiShell::GuiShell(NCursesSink &buff) :
    std::ostream(&buff)
{
    // Use the terminal itself, rather than stdin and stdout, which may be pipes (e.g. a tree read
    // from stdin, and picked paths written to stdout)
    FILE* terminal = fopen("/dev/tty", "r+");
    if (terminal == nullptr) {
        throw std::runtime_error(std::string("Cannot open the terminal (/dev/tty): ") +
                                 strerror(errno));
    }
    _terminal_fd = fileno(terminal);
    set_term(newterm(nullptr, terminal, terminal));
    // Get each key as it's pressed (Ctrl-C included, as a key rather than a signal), without
    // echoing it onto the screen
    raw();
    noecho();
    // Get arrow keys and the like as single key codes (KEY_UP...), rather than as escape
    // sequences, which would otherwise end up typed into the search keyword
    keypad(stdscr, TRUE);
    // With keypad mode, curses waits after Esc in case it starts a key's escape sequence. The
    // default wait is a second, which makes Esc feel stuck; terminals send sequences at once.
    set_escdelay(25);
    start_color();
    use_default_colors();
    is_initialized = true;
    for (auto& style : STYLES) {
        init_pair(color_pair(style.style), style.foreground, style.background);
    }
}

GuiShell& GuiShell::get_instance(void) {
    static NCursesSink sink;
    static GuiShell instance(sink);
    return instance;
}

GuiShell::~GuiShell(void) {
    is_initialized = false;
    endwin();
}

void GuiShell::refresh(void) {
    flush();
    ::refresh();
}

void GuiShell::clear(void) {
    erase(); // ncurses call
    is_screen_full = false;
}

bool GuiShell::is_initialized = false;

Color::Color(Style style) : _style(style)
{}

std::ostream& operator<< (std::ostream &out, const Color &color) {
    if (GuiShell::is_initialized) {
        out.flush();
        attron(COLOR_PAIR(color_pair(color._style)));
    } else {
        out << "\e[" << STYLES[color._style].ansi_code << "m";
    }
    return out;
}


std::ostream& operator<< (std::ostream &out, const Bold &bold) {
    if (GuiShell::is_initialized) {
        out.flush();
        attron(A_BOLD);
    } else {
        out << "\033[1m";
    }
    return out;
}

std::ostream& operator<< (std::ostream &out, const Unbold &unbold) {
    if (GuiShell::is_initialized) {
        out.flush();
        attroff(A_BOLD);
    } else {
        out << "\033[0m";
    }
    return out;
}

}
