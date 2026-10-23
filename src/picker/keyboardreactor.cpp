#include <functional>
#include <curses.h>

#include "picker/keyboardreactor.h"

namespace picker {

KeyboardReactor::KeyboardReactor(std::function<void(void)> initialize_callback,
                                    std::function<bool(int)> char_pressed_callback):
    _initialize_callback(initialize_callback),
    _char_pressed_callback(char_pressed_callback)
{
}

void KeyboardReactor::run(void) {
    _initialize_callback();

    // Keys are handled one at a time on this thread, so a slow redraw can't race with the next
    // key (a separate reading thread once passed keys by reference and lost some of them)
    bool is_finished = false;
    while (not is_finished) {
        const int c = getch();
        // ERR means stdin is gone (e.g. the terminal was closed)
        if (c == ERR) {
            break;
        }
        is_finished = _char_pressed_callback(c);
    }
    endwin();
}

}
