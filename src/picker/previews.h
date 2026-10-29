#ifndef PICKER_PREVIEWS_H__
#define PICKER_PREVIEWS_H__

#include <string>
#include <utility>
#include <vector>

namespace picker {

// The lines to show beside the tree while a node is selected (--preview FD), e.g. what the caller
// knows about it that the tree doesn't show
struct Preview {
    // The node's path, as printed; "": the preview of the nodes that have none of their own (e.g.
    // that the caller is still working them out)
    std::string path;
    std::vector<std::string> lines;

    bool operator==(const Preview& other) const {
        return path == other.path and lines == other.lines;
    }
};

// Previews read from a file descriptor while picking. Each is the node's path, a tab, and its
// lines, separated by tabs, and ends with the delimiter (so it's one line without -0); a later
// one for a path replaces the earlier.
class PreviewsReader {
public:
    explicit PreviewsReader(char delimiter);

    // Take some of what was read (a preview may be split between reads), and return the previews
    // it completes, in order. Ones without a tab are skipped.
    std::vector<Preview> feed(const std::string& data);

private:
    char _delimiter;
    // The start of a preview whose end hasn't been read yet
    std::string _pending;
};

} // namespace picker

#endif
