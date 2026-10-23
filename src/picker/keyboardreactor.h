#ifndef KEYBOARD_REACTOR_H__
#define KEYBOARD_REACTOR_H__

#include <functional>

namespace picker {

class KeyboardReactor
{
public:
    KeyboardReactor(std::function<void(void)> initialize_callback,
                    std::function<bool(int)> char_pressed_callback);

    // Read keys from the terminal and pass each to the callback, until it returns true
    // (or input ends)
    void run(void);

private:
    std::function<void(void)> _initialize_callback;
    std::function<bool(int)> _char_pressed_callback;
};

}

#endif
