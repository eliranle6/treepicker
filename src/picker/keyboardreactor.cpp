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
                                 std::vector<DataSource> data_sources):
    _initialize_callback(initialize_callback),
    _char_pressed_callback(char_pressed_callback)
{
    for (auto& source : data_sources) {
        if (source.fd >= 0) {
            _data_sources.push_back(std::move(source));
        }
    }
}

bool KeyboardReactor::wait_for_key(void) {
    while (not _data_sources.empty()) {
        // curses reads keys from the terminal a byte at a time, so it has none waiting unread
        // when the terminal has nothing to read
        std::vector<pollfd> fds = {{guishell::GuiShell::get_instance().terminal_fd(), POLLIN, 0}};
        for (auto& source : _data_sources) {
            fds.push_back({source.fd, POLLIN, 0});
        }
        if (poll(fds.data(), fds.size(), -1) < 0) {
            // E.g. a resize's signal: let getch() say so (KEY_RESIZE)
            return errno == EINTR;
        }
        // From the last, so dropping one that ended keeps the others' places
        for (std::size_t i = _data_sources.size(); i-- > 0;) {
            if (fds[i + 1].revents == 0) {
                continue;
            }
            char buffer[4096];
            const ssize_t size = read(_data_sources[i].fd, buffer, sizeof(buffer));
            if (size > 0) {
                _data_sources[i].data_read_callback(std::string(buffer, size));
            } else {
                // It ended (or broke)
                _data_sources.erase(_data_sources.begin() + i);
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
