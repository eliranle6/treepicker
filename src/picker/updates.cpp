#include "picker/updates.h"

namespace picker {

UpdatesReader::UpdatesReader(char delimiter) :
    _delimiter(delimiter)
{}

std::vector<Update> UpdatesReader::feed(const std::string& data) {
    static const std::string DISABLED = "\tdisabled";
    std::vector<Update> updates;
    _pending += data;
    std::size_t start = 0;
    for (std::size_t end; (end = _pending.find(_delimiter, start)) != std::string::npos;
         start = end + 1) {
        const std::string update = _pending.substr(start, end - start);
        const std::size_t tab = update.find('\t');
        if (tab != std::string::npos) {
            std::string tag = update.substr(tab + 1);
            // A tag can't end with it: tabs are shown as '?' anyway
            const bool is_disabled = tag.size() >= DISABLED.size() and
                tag.compare(tag.size() - DISABLED.size(), DISABLED.size(), DISABLED) == 0;
            if (is_disabled) {
                tag.resize(tag.size() - DISABLED.size());
            }
            updates.push_back({update.substr(0, tab), tag, is_disabled});
        }
    }
    _pending.erase(0, start);
    return updates;
}

} // namespace picker
