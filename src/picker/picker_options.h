#ifndef PICKER_OPTIONS_H__
#define PICKER_OPTIONS_H__

#include <string>
#include <vector>

namespace picker {

struct PickerOptions {
    // Pick several nodes: Space picks or unpicks, Enter finishes
    bool is_multi = false;
    // Lines shown above the tree
    std::string header;
    // A label at the start of the stats line, in bold, on label_color (see
    // guishell::label_color_pair), e.g. a mode the caller is in
    std::string label;
    std::string label_color = "blue";
    // Start with the root's first child selected, rather than the root
    bool is_first_child_selected_first = false;
    // Whether Enter picks the root (it may be just a title, e.g. of a list of options)
    bool is_root_pickable = true;
    // With is_multi, the ids of the nodes to start picked (each on its own, not with its subtree)
    std::vector<std::string> initially_picked_ids;
    // A file descriptor to read changes to the nodes' tags from while picking (see
    // UpdatesReader), each ending with updates_delimiter; -1: none
    int updates_fd = -1;
    char updates_delimiter = '\n';
};

} // namespace picker

#endif
