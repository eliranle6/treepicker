#ifndef PICKER_OPTIONS_H__
#define PICKER_OPTIONS_H__

#include <string>

namespace picker {

struct PickerOptions {
    // Pick several nodes: Space picks or unpicks, Enter finishes
    bool is_multi = false;
    // Lines shown above the tree
    std::string header;
};

} // namespace picker

#endif
