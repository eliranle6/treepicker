#ifndef PICKER_OPTIONS_H__
#define PICKER_OPTIONS_H__

#include <string>

namespace picker {

struct PickerOptions {
    // Pick several nodes: Space picks or unpicks, Enter finishes
    bool is_multi = false;
    // Lines shown above the tree
    std::string header;
    // Start with the root's first child selected, rather than the root
    bool is_first_child_selected_first = false;
    // Whether Enter picks the root (it may be just a title, e.g. of a list of options)
    bool is_root_pickable = true;
};

} // namespace picker

#endif
