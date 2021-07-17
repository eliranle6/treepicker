#ifndef KEYBOARD_REACTOR_H__
#define KEYBOARD_REACTOR_H__

#include <functional>
#include <thread>

#include <boost/asio/io_service.hpp>

namespace picker {

class KeyboardReactor
{
public:
    KeyboardReactor(std::function<void(void)> initialize_callback,
                    std::function<bool(char)> char_pressed_callback);

    void run(void);

    void trigger_from_stdin(boost::asio::io_service *io_service);

private:
    boost::asio::io_service _io_service;
    std::function<void(void)> _initialize_callback;
    std::function<bool(char)> _char_pressed_callback;
};

}

#endif
