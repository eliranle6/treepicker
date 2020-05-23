#include <streambuf>
#include <iostream>
#include <curses.h>
#include <unistd.h>
#include <boost/iostreams/stream.hpp>

#include "guishell.h"

char OUTPUT_BUFFER[1024];

namespace guishell {

std::streamsize NCursesSink::write(const char *s, std::streamsize n) {
    strncpy(OUTPUT_BUFFER, s, sizeof(OUTPUT_BUFFER));
    OUTPUT_BUFFER[n] = '\0';
    printw(OUTPUT_BUFFER);
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

    init_pair(NON_SELECTED_DIRECTORY, DIRECTORY, -1);
    init_pair(SELECTED_DIRECTORY, DIRECTORY, COLOR_BLUE);

    init_pair(WHITE_ON_BLUE, COLOR_WHITE, COLOR_BLUE);
    init_pair(BLACK_ON_BLUE, COLOR_BLACK, COLOR_BLUE);
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
