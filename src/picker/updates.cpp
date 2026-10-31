#include "picker/updates.h"

#include "utils/guishell.h"

namespace picker {

UpdatesReader::UpdatesReader(char delimiter) :
    _delimiter(delimiter)
{}

std::vector<Update> UpdatesReader::feed(const std::string& data) {
    static const std::string DISABLED = "\tdisabled";
    static const std::string COLOR = "\tcolor=";
    static const std::string SKIPPED = "\tskipped";
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
            auto ends_with = [&tag](const std::string& end) {
                return tag.size() >= end.size() and tag.compare(tag.size() - end.size(), end.size(), end) == 0;
            };
            const bool is_skipped = ends_with(SKIPPED);
            const bool is_disabled = is_skipped or ends_with(DISABLED);
            if (is_disabled) {
                tag.resize(tag.size() - (is_skipped ? SKIPPED : DISABLED).size());
            }
            std::string color;
            const std::size_t color_start = tag.rfind(COLOR);
            if (color_start != std::string::npos and
                    tag.find('\t', color_start + COLOR.size()) == std::string::npos) {
                color = tag.substr(color_start + COLOR.size());
                tag.resize(color_start);
            }
            updates.push_back({update.substr(0, tab), tag, is_disabled, color, is_skipped});
        }
    }
    _pending.erase(0, start);
    return updates;
}

std::vector<Label> parse_status(const std::string& status) {
    static const std::string DEFAULT_COLOR = "yellow";
    if (status.empty()) {
        return {};
    }
    if (status[0] != STATUS_MARK) {
        return {{status, DEFAULT_COLOR}};
    }
    std::vector<Label> labels;
    // Each part: the mark, its color, the mark, its text (up to the next part's mark)
    for (std::size_t start = 0; start < status.size();) {
        const std::size_t color_end = status.find(STATUS_MARK, start + 1);
        if (color_end == std::string::npos) {
            break;
        }
        std::size_t text_end = status.find(STATUS_MARK, color_end + 1);
        if (text_end == std::string::npos) {
            text_end = status.size();
        }
        const std::string color = status.substr(start + 1, color_end - start - 1);
        const std::string text = status.substr(color_end + 1, text_end - color_end - 1);
        if (not text.empty()) {
            labels.push_back({text, guishell::is_label_color(color) ? color : DEFAULT_COLOR});
        }
        start = text_end;
    }
    return labels;
}

} // namespace picker
