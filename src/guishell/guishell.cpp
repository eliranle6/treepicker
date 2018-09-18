#include <streambuf>
#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <boost/iostreams/stream.hpp>

#include "guishell.h"

namespace guishell {

std::streamsize NCursesSink::write(const char *s, std::streamsize n) {
    const char *bound = s + n - 1;
    for(; s <= bound; ++s) {
        addch(*s);
    }
    return n;
}

GuiShell::GuiShell(NCursesSink &buff) :
    GuiShellStream(buff)
{
    initscr();
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

bool GuiShell::is_initialized = false;

Color::Color(int code) : _code(code) {
}

std::ostream& operator<< (std::ostream &out, const Color &color) {
    if (GuiShell::is_initialized) {
        out.flush();
        attron(COLOR_PAIR(color._code));
    } else {
        out << "\e[" << color._code << "m";
    }
    return out;
}

}
