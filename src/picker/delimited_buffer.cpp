#include "picker/delimited_buffer.h"

namespace picker {

DelimitedBuffer::DelimitedBuffer(char delimiter) :
    _delimiter(delimiter)
{}

std::vector<std::string> DelimitedBuffer::feed(const std::string& data) {
    std::vector<std::string> entries;
    _pending += data;
    std::size_t start = 0;
    for (std::size_t end; (end = _pending.find(_delimiter, start)) != std::string::npos;
         start = end + 1) {
        entries.push_back(_pending.substr(start, end - start));
    }
    _pending.erase(0, start);
    return entries;
}

} // namespace picker
