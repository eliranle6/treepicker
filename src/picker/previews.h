#ifndef PICKER_PREVIEWS_H__
#define PICKER_PREVIEWS_H__

#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

namespace picker {

// Lines to show beside the tree when a node is selected, by the node's path (--preview FILE),
// e.g. what the caller knows about it that the tree doesn't show
using Previews = std::unordered_map<std::string, std::vector<std::string>>;

// Read previews from `in`: each is the node's path, a tab, and its lines, separated by tabs, and
// ends with the delimiter (so it's one line of the file without -0). Ones without a tab are
// skipped; a path given again has its last preview.
Previews read_previews(std::istream& in, char delimiter);

} // namespace picker

#endif
