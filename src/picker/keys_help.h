#ifndef PICKER_KEYS_HELP_H__
#define PICKER_KEYS_HELP_H__

#include <memory>
#include <string>
#include <vector>

#include "picker/picker_options.h"

namespace picker {

// A line of the keys' help: what it's about (a heading, e.g. "Moving"), the keys, and what they
// do (one line, wrapped where it's shown)
struct KeyLine {
    std::string group;
    std::string keys;
    std::string description;
};

// All of treepicker's keys: TreeKeyboardSelector::get_key_commands(), and the ones it handles
// apart (while typing a search, in the side tree, Tab), by group, in the groups' order
std::vector<KeyLine> key_lines(void);

// The picker's keys, by group, a line each: shown by --help
std::string keys_help(void);

// The keys' help that the picker shows (the ? key), as a list to move through and search: the
// caller's keys and lines first (options' key_hints, help_lines, in which one that isn't indented
// and ends with ':' is a heading, and side_key_hints), then
// key_lines() with the states' keys, each group after a heading that moving skips. Its lines are
// wrapped to width columns (73: an 80-column screen's, after the tree's "> ├── " and the last
// column, left empty).
std::unique_ptr<treelib::Tree> keys_help_tree(const PickerOptions& options, std::size_t width = 73);

} // namespace picker

#endif
