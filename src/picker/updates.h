#ifndef PICKER_UPDATES_H__
#define PICKER_UPDATES_H__

#include <string>
#include <utility>
#include <vector>

#include "picker/picker_options.h"

namespace picker {

// A change to a node: its new tag, and whether it's disabled from now on
struct Update {
    std::string id;
    std::string tag;
    bool is_disabled;

    bool operator==(const Update& other) const {
        return id == other.id and tag == other.tag and is_disabled == other.is_disabled;
    }
};

// Changes to the shown names (tags) of the tree's nodes while it's shown, read from a file
// descriptor (--updates FD), e.g. a menu's options saying how something they're about is going.
// Each is "ID\tTAG", or "ID\tTAG\tdisabled" for a node that can't be picked (until an update
// without it), and ends with the delimiter; the node keeps its id, so it's printed as before when
// picked. One with an empty ID is the caller's status (what it's doing, e.g. working something out
// about the nodes), shown as labels on the stats line ("": none; see parse_status()).
class UpdatesReader {
public:
    explicit UpdatesReader(char delimiter);

    // Take some of what was read (an update may be split between reads), and return the updates
    // it completes, in order. Ones without a tab are skipped.
    std::vector<Update> feed(const std::string& data);

private:
    char _delimiter;
    // The start of an update whose end hasn't been read yet
    std::string _pending;
};

// The labels of a status (an update with an empty ID): one yellow label of the whole text, or, if
// it starts with STATUS_MARK, a label for each part starting with STATUS_MARK, a label color's name
// and STATUS_MARK again (as a preview's highlighted line does), in that color (yellow, if it's not
// one). Empty ones are dropped.
const char STATUS_MARK = '\x01';
std::vector<Label> parse_status(const std::string& status);

} // namespace picker

#endif
