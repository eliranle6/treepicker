#include "picker/previews.h"

namespace picker {

PreviewLine parse_preview_line(const std::string& line) {
    if (not line.empty() and line[0] == HIGHLIGHT_MARK) {
        const std::size_t end = line.find(HIGHLIGHT_MARK, 1);
        if (end != std::string::npos) {
            return {line.substr(end + 1), line.substr(1, end - 1)};
        }
    }
    return {line, ""};
}

std::vector<PreviewLine> parse_preview_segments(const std::string& line) {
    std::vector<PreviewLine> segments;
    std::string color;
    for (std::size_t start = 0; start < line.size();) {
        const std::size_t mark = line.find(HIGHLIGHT_MARK, start);
        const std::size_t end = mark == std::string::npos ? line.size() : mark;
        if (end > start) {
            segments.push_back({line.substr(start, end - start), color});
        }
        if (mark == std::string::npos) {
            break;
        }
        const std::size_t color_end = line.find(HIGHLIGHT_MARK, mark + 1);
        if (color_end == std::string::npos) {
            // A mark without its color's end: the rest is text
            segments.push_back({line.substr(mark + 1), color});
            break;
        }
        color = line.substr(mark + 1, color_end - mark - 1);
        start = color_end + 1;
    }
    return segments;
}

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
