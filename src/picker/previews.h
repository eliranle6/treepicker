#ifndef PICKER_PREVIEWS_H__
#define PICKER_PREVIEWS_H__

#include <string>
#include <utility>
#include <vector>

#include "picker/delimited_buffer.h"
#include "picker/image.h"
#include "picker/markup.h"

namespace picker {

// The lines to show beside the tree while a node is selected (--preview FD), e.g. what the caller
// knows about it that the tree doesn't show
struct Preview {
    // The node's path, as printed; "": the preview of the nodes that have none of their own (e.g.
    // that the caller is still working them out)
    std::string path;
    std::vector<std::string> lines;
    // Whether it's the node's image rather than its lines (see IMAGE_MARK), and the image (an
    // empty one: none, no longer)
    bool is_image = false;
    Image image;

    bool operator==(const Preview& other) const {
        return path == other.path and lines == other.lines and is_image == other.is_image and
            image == other.image;
    }
};

// An entry starting with it is an image's: the mark, the node's path, a tab, and the image (see
// parse_image()), shown in the column below the node's lines, in the rows they leave
constexpr char IMAGE_MARK = '\x03';

// Previews read from a file descriptor while picking. Each is the node's path, a tab, and its
// lines, separated by tabs, and ends with the delimiter (so it's one line without -0); a later
// one for a path replaces the earlier. Or an image's (IMAGE_MARK), which replaces the path's image.
class PreviewsReader {
public:
    explicit PreviewsReader(char delimiter);

    // Take some of what was read (a preview may be split between reads), and return the previews
    // it completes, in order. Ones without a tab are skipped.
    std::vector<Preview> feed(const std::string& data);

private:
    DelimitedBuffer _buffer;
};

} // namespace picker

#endif
