#ifndef PICKER_UPDATES_H__
#define PICKER_UPDATES_H__

#include <string>
#include <utility>
#include <vector>

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
// picked.
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

} // namespace picker

#endif
