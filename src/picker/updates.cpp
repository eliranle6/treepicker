#include "picker/updates.h"

namespace picker {

UpdatesReader::UpdatesReader(char delimiter) :
    _buffer(delimiter)
{}

std::vector<Update> UpdatesReader::feed(const std::string& data) {
    static const std::string DISABLED = "\tdisabled";
    static const std::string COLOR = "\tcolor=";
    static const std::string SKIPPED = "\tskipped";
    static const std::string STATE = "\tstate=";
    std::vector<Update> updates;
    for (const std::string& update : _buffer.feed(data)) {
        const std::size_t tab = update.find('\t');
        if (tab != std::string::npos) {
            std::string tag = update.substr(tab + 1);
            // A tag can't end with it: tabs are shown as '?' anyway
            auto ends_with = [&tag](const std::string& end) {
                return tag.size() >= end.size() and tag.compare(tag.size() - end.size(), end.size(), end) == 0;
            };
            const bool is_skipped = ends_with(SKIPPED);
            const bool is_disabled = is_skipped or ends_with(DISABLED);
            if (is_disabled) {
                tag.resize(tag.size() - (is_skipped ? SKIPPED : DISABLED).size());
            }
            std::string state;
            const std::size_t state_start = tag.rfind(STATE);
            if (state_start != std::string::npos and
                    tag.find('\t', state_start + STATE.size()) == std::string::npos) {
                state = tag.substr(state_start + STATE.size());
                tag.resize(state_start);
            }
            std::string color;
            const std::size_t color_start = tag.rfind(COLOR);
            if (color_start != std::string::npos and
                    tag.find('\t', color_start + COLOR.size()) == std::string::npos) {
                color = tag.substr(color_start + COLOR.size());
                tag.resize(color_start);
            }
            updates.push_back({update.substr(0, tab), tag, is_disabled, color, is_skipped, state});
        }
    }
    return updates;
}

} // namespace picker
