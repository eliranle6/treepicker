#ifndef PICKER_UPDATES_H__
#define PICKER_UPDATES_H__

#include <string>
#include <utility>
#include <vector>

namespace picker {

// Changes to the shown names (tags) of the tree's nodes while it's shown, read from a file
// descriptor (--updates FD), e.g. a menu's options saying how something they're about is going.
// Each is "ID\tTAG" and ends with the delimiter; the node keeps its id, so it's printed as before
// when picked.
class UpdatesReader {
public:
    explicit UpdatesReader(char delimiter);

    // Take some of what was read (an update may be split between reads), and return the updates
    // it completes, as (id, tag), in order. Ones without a tab are skipped.
    std::vector<std::pair<std::string, std::string>> feed(const std::string& data);

private:
    char _delimiter;
    // The start of an update whose end hasn't been read yet
    std::string _pending;
};

} // namespace picker

#endif
