#ifndef KEYBOARD_REACTOR_H__
#define KEYBOARD_REACTOR_H__

#include <functional>
#include <string>
#include <vector>

namespace picker {

class KeyboardReactor
{
public:
    // A file descriptor to read data from while reading keys, passing what's read to its
    // callback, as it comes, until it ends
    struct DataSource {
        int fd;
        std::function<void(const std::string&)> data_read_callback;
    };

    // data_sources with an fd of -1 are skipped
    KeyboardReactor(std::function<void(void)> initialize_callback,
                    std::function<bool(int)> char_pressed_callback,
                    std::vector<DataSource> data_sources = {});

    // Read keys from the terminal and pass each to the callback, until it returns true
    // (or input ends)
    void run(void);

private:
    // Wait until a key or a data source's data comes, and pass the data on. Returns whether a key
    // came.
    bool wait_for_key(void);

    std::function<void(void)> _initialize_callback;
    std::function<bool(int)> _char_pressed_callback;
    // The ones that haven't ended
    std::vector<DataSource> _data_sources;
};

}

#endif
