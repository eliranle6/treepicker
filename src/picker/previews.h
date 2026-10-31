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

// A preview's line, as shown: its text, and the color of its highlight (a label color, see
// guishell::label_color_pair: bold, on that color, across the column), or "" for none
struct PreviewLine {
    std::string text;
    std::string highlight;
};

// How a preview's line is shown: one starting with HIGHLIGHT_MARK, a label color's name and
// HIGHLIGHT_MARK again is highlighted in that color (the rest of it is its text); else it's as it is
constexpr char HIGHLIGHT_MARK = '\x01';
PreviewLine parse_preview_line(const std::string& line);

// A preview's line shown below the tree (--preview-below), in parts: plain text, until
// HIGHLIGHT_MARK, a label color's name and HIGHLIGHT_MARK again, after which it's shown on that
// color (bold), until the next such mark (an empty color's: plain again). Empty parts are dropped.
std::vector<PreviewLine> parse_preview_segments(const std::string& line);

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
