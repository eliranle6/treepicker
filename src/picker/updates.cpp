#include "picker/updates.h"

namespace picker {

UpdatesReader::UpdatesReader(char delimiter) :
    _delimiter(delimiter)
{}

std::vector<std::pair<std::string, std::string>> UpdatesReader::feed(const std::string& data) {
    std::vector<std::pair<std::string, std::string>> updates;
    _pending += data;
    std::size_t start = 0;
    for (std::size_t end; (end = _pending.find(_delimiter, start)) != std::string::npos;
         start = end + 1) {
        const std::string update = _pending.substr(start, end - start);
        const std::size_t tab = update.find('\t');
        if (tab != std::string::npos) {
            updates.emplace_back(update.substr(0, tab), update.substr(tab + 1));
        }
    }
    _pending.erase(0, start);
    return updates;
}

} // namespace picker
