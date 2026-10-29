#include "picker/previews.h"

namespace picker {

PreviewsReader::PreviewsReader(char delimiter) :
    _delimiter(delimiter)
{}

std::vector<Preview> PreviewsReader::feed(const std::string& data) {
    std::vector<Preview> previews;
    _pending += data;
    std::size_t start = 0;
    for (std::size_t end; (end = _pending.find(_delimiter, start)) != std::string::npos;
         start = end + 1) {
        const std::string entry = _pending.substr(start, end - start);
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
    _pending.erase(0, start);
    return previews;
}

} // namespace picker
