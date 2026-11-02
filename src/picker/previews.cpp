#include "picker/previews.h"

namespace picker {

PreviewsReader::PreviewsReader(char delimiter) :
    _buffer(delimiter)
{}

std::vector<Preview> PreviewsReader::feed(const std::string& data) {
    std::vector<Preview> previews;
    for (const std::string& entry : _buffer.feed(data)) {
        std::size_t tab = entry.find('\t');
        if (tab == std::string::npos) {
            continue;
        }
        Preview preview{entry.substr(0, tab), {}};
        for (std::size_t line_start = tab + 1;; line_start = tab + 1) {
            tab = entry.find('\t', line_start);
            preview.lines.push_back(entry.substr(line_start, tab == std::string::npos ?
                                                 std::string::npos : tab - line_start));
            if (tab == std::string::npos) {
                break;
            }
        }
        previews.push_back(std::move(preview));
    }
    return previews;
}

} // namespace picker
