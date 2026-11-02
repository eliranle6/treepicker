#ifndef PICKER_UPDATES_H__
#define PICKER_UPDATES_H__

#include <string>
#include <utility>
#include <vector>

#include "picker/delimited_buffer.h"
#include "picker/markup.h"
#include "picker/picker_options.h"

namespace picker {

// A change to a node: its new tag, and whether it's disabled from now on
struct Update {
    std::string id;
    std::string tag;
    bool is_disabled;
    // Its text's color (see Node::color), "" for the default
    std::string color;
    // Whether moving skips it, too (e.g. a menu's heading or blank line): disabled, and not where
    // the selection stops
    bool is_skipped = false;

    bool operator==(const Update& other) const {
        return id == other.id and tag == other.tag and is_disabled == other.is_disabled and
            color == other.color and is_skipped == other.is_skipped;
    }
};

// Changes to the shown names (tags) of the tree's nodes while it's shown, read from a file
// descriptor (--updates FD), e.g. a menu's options saying how something they're about is going.
// Each is "ID\tTAG", then optionally "\tcolor=NAME" for its text's color, and "\tdisabled" for a
// node that can't be picked (until an update without it), or "\tskipped" for one that moving skips
// too, and ends with the delimiter; the node keeps its id, so it's printed as before when
// picked. One with an empty ID is the caller's status (what it's doing, e.g. working something out
// about the nodes), shown as labels on the stats line ("": none; see parse_status()).
class UpdatesReader {
public:
    explicit UpdatesReader(char delimiter);

    // Take some of what was read (an update may be split between reads), and return the updates
    // it completes, in order. Ones without a tab are skipped.
    std::vector<Update> feed(const std::string& data);

private:
    DelimitedBuffer _buffer;
};

} // namespace picker

#endif
