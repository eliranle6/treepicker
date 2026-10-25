#ifndef KEYBOARD_REACTOR_H__
#define KEYBOARD_REACTOR_H__

#include <functional>
#include <string>

namespace picker {

class KeyboardReactor
{
public:
    // With data_fd (-1: none), also pass what's read from it to data_read_callback, as it comes,
    // until it ends
    KeyboardReactor(std::function<void(void)> initialize_callback,
                    std::function<bool(int)> char_pressed_callback,
                    int data_fd = -1,
                    std::function<void(const std::string&)> data_read_callback = nullptr);

    // Read keys from the terminal and pass each to the callback, until it returns true
    // (or input ends)
    void run(void);

private:
    // Wait until a key or data_fd's data comes, and pass data_fd's on. Returns whether a key came.
    bool wait_for_key(void);

    std::function<void(void)> _initialize_callback;
    std::function<bool(int)> _char_pressed_callback;
    int _data_fd;
    std::function<void(const std::string&)> _data_read_callback;
};

}

#endif
