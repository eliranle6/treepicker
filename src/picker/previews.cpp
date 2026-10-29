#include "picker/previews.h"

namespace picker {

Previews read_previews(std::istream& in, char delimiter) {
    Previews previews;
    for (std::string entry; std::getline(in, entry, delimiter);) {
        std::size_t tab = entry.find('\t');
        if (tab == std::string::npos) {
            continue;
        }
        std::vector<std::string> lines;
        for (std::size_t start = tab + 1;; start = tab + 1) {
            tab = entry.find('\t', start);
            lines.push_back(entry.substr(start, tab == std::string::npos ? std::string::npos :
                                                tab - start));
            if (tab == std::string::npos) {
                break;
            }
        }
        previews[entry.substr(0, entry.find('\t'))] = std::move(lines);
    }
    return previews;
}

} // namespace picker
