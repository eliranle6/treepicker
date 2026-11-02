#include "picker/markup.h"

#include "utils/guishell.h"

namespace picker {

std::vector<Label> parse_segments(const std::string& text) {
    std::vector<Label> segments;
    std::string color;
    for (std::size_t start = 0; start < text.size();) {
        const std::size_t mark = text.find(MARK, start);
        const std::size_t end = mark == std::string::npos ? text.size() : mark;
        if (end > start) {
            segments.push_back({text.substr(start, end - start), color});
        }
        if (mark == std::string::npos) {
            break;
        }
        const std::size_t color_end = text.find(MARK, mark + 1);
        if (color_end == std::string::npos) {
            if (mark + 1 < text.size()) {
                segments.push_back({text.substr(mark + 1), color});
            }
            break;
        }
        color = text.substr(mark + 1, color_end - mark - 1);
        start = color_end + 1;
    }
    return segments;
}

std::vector<Label> parse_status(const std::string& status) {
    static const std::string DEFAULT_COLOR = "yellow";
    std::vector<Label> labels = parse_segments(status);
    for (auto& label : labels) {
        if (not guishell::is_label_color(label.color)) {
            label.color = DEFAULT_COLOR;
        }
    }
    return labels;
}

Label parse_preview_line(const std::string& line) {
    Label shown{"", ""};
    const std::vector<Label> segments = parse_segments(line);
    if (not segments.empty()) {
        shown.color = segments[0].color;
    }
    for (auto& segment : segments) {
        shown.text += segment.text;
    }
    return shown;
}

} // namespace picker
