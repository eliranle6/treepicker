#ifndef PICKER_DELIMITED_BUFFER_H__
#define PICKER_DELIMITED_BUFFER_H__

#include <string>
#include <vector>

namespace picker {

// What's read from a file descriptor while picking, in entries that each end with a delimiter, and
// may be split between reads
class DelimitedBuffer {
public:
    explicit DelimitedBuffer(char delimiter);

    // Take some of what was read, and return the entries it completes, in order
    std::vector<std::string> feed(const std::string& data);

private:
    char _delimiter;
    // The start of an entry whose end hasn't been read yet
    std::string _pending;
};

} // namespace picker

#endif
