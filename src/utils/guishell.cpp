#include <algorithm>
#include <cwchar>
#include <streambuf>
#include <iostream>
#include <curses.h>
#include <unistd.h>
#include <boost/iostreams/stream.hpp>

#include "guishell.h"

namespace guishell {

std::size_t nr_bytes_that_fit(const char *s, std::size_t n, int nr_columns) {
    std::mbstate_t state = std::mbstate_t();
    std::size_t nr_bytes = 0;
    int nr_columns_used = 0;
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
        if (nr_columns_used + char_width > nr_columns) {
            break;
        }
        nr_columns_used += char_width;
        nr_bytes += char_size;
    }
    return nr_bytes;
}

std::streamsize NCursesSink::write(const char *s, std::streamsize n) {
    // Print line by line, cutting each line at the right edge of the screen, since a wrapped line
    // would push the rest of the tree down. The last column is left empty, since writing to it
    // moves the cursor to the next line. Not printw: text may contain '%'.
    const char *end = s + n;
    while (s < end) {
        const char *newline = std::find(s, end, '\n');
        const int nr_columns_left = COLS - 1 - getcurx(stdscr);
        if (nr_columns_left > 0) {
            addnstr(s, nr_bytes_that_fit(s, newline - s, nr_columns_left));
        }
        if (newline != end) {
            addch('\n');
            s = newline + 1;
        } else {
            s = end;
        }
    }
    return n;
}

GuiShell::GuiShell(NCursesSink &buff) :
    GuiShellStream(buff)
{
    initscr();
    // Get each key as it's pressed (Ctrl-C included, as a key rather than a signal), without
    // echoing it onto the screen
    raw();
    noecho();
    // Get arrow keys and the like as single key codes (KEY_UP...), rather than as escape
    // sequences, which would otherwise end up typed into the search keyword
    keypad(stdscr, TRUE);
    start_color();
    use_default_colors();
    is_initialized = true;
    init_pair(DEFAULT, -1, -1);
    init_pair(BLACK, COLOR_BLACK, -1);
    init_pair(RED, COLOR_RED, -1);
    init_pair(GREEN, COLOR_GREEN, -1);
    init_pair(YELLOW, COLOR_YELLOW, -1);
    init_pair(BLUE, COLOR_BLUE, -1);
    init_pair(MAGENTA, COLOR_MAGENTA, -1);
    init_pair(CYAN, COLOR_CYAN, -1);
    init_pair(WHITE, COLOR_WHITE, -1);

    init_pair(NON_SELECTED_DIRECTORY, DIRECTORY, -1);
    init_pair(SELECTED_DIRECTORY, DIRECTORY, COLOR_BLUE);

    init_pair(WHITE_ON_BLUE, COLOR_WHITE, COLOR_BLUE);
    init_pair(BLACK_ON_BLUE, COLOR_BLACK, COLOR_BLUE);
    init_pair(BLACK_ON_WHITE, COLOR_BLACK, COLOR_WHITE);
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
}

bool GuiShell::is_initialized = false;

Color::Color(int code) : _code(code)
{}

std::ostream& operator<< (std::ostream &out, const Color &color) {
    if (GuiShell::is_initialized) {
        out.flush();
        attron(COLOR_PAIR(color._code));
    } else {
        out << "\e[" << color._code << "m";
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
