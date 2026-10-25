#include <cerrno>
#include <functional>
#include <curses.h>
#include <poll.h>
#include <unistd.h>

#include "picker/keyboardreactor.h"
#include "utils/guishell.h"

namespace picker {

KeyboardReactor::KeyboardReactor(std::function<void(void)> initialize_callback,
                                    std::function<bool(int)> char_pressed_callback,
                                    int data_fd,
                                    std::function<void(const std::string&)> data_read_callback):
    _initialize_callback(initialize_callback),
    _char_pressed_callback(char_pressed_callback),
    _data_fd(data_fd),
    _data_read_callback(data_read_callback)
{
}

bool KeyboardReactor::wait_for_key(void) {
    while (_data_fd >= 0) {
        // curses reads keys from the terminal a byte at a time, so it has none waiting unread
        // when the terminal has nothing to read
        pollfd fds[] = {{guishell::GuiShell::get_instance().terminal_fd(), POLLIN, 0},
                        {_data_fd, POLLIN, 0}};
        if (poll(fds, 2, -1) < 0) {
            // E.g. a resize's signal: let getch() say so (KEY_RESIZE)
            return errno == EINTR;
        }
        if (fds[1].revents != 0) {
            char buffer[4096];
            const ssize_t size = read(_data_fd, buffer, sizeof(buffer));
            if (size > 0) {
                _data_read_callback(std::string(buffer, size));
            } else {
                // It ended (or broke): keys only from now on
                _data_fd = -1;
            }
        }
        if (fds[0].revents != 0) {
            return true;
        }
    }
    return true;
}

void KeyboardReactor::run(void) {
    _initialize_callback();

    // Keys are handled one at a time on this thread, so a slow redraw can't race with the next
    // key (a separate reading thread once passed keys by reference and lost some of them)
    bool is_finished = false;
    while (not is_finished and wait_for_key()) {
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
