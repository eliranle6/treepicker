#ifndef PICKER_OPTIONS_H__
#define PICKER_OPTIONS_H__

#include <string>
#include <vector>

namespace picker {

// A label at the start of the stats line, in bold, on its color (see guishell::label_color_pair),
// e.g. a mode the caller is in
struct Label {
    std::string text;
    std::string color = "blue";
};

// A key the keys row at the bottom names, with what it does here, e.g. {"Space", "pick"}
struct KeyHint {
    std::string key;
    std::string text;

    bool operator==(const KeyHint& other) const {
        return key == other.key and text == other.text;
    }
};

struct PickerOptions {
    // Pick several nodes: Space picks or unpicks, Enter finishes
    bool is_multi = false;
    // With is_multi, Enter with nothing picked picks the selected node (and what's under it, as
    // Space does), rather than doing nothing
    bool is_selection_picked_if_none = false;
    // Lines shown above the tree
    std::string header;
    // The labels at the start of the stats line, side by side
    std::vector<Label> labels;
    // The caller's keys, for the keys row (see key_hints()) and the help (?): its own (e.g. an
    // --expect key), or treepicker's with what they do here (e.g. {"Enter", "choose what to do"})
    std::vector<KeyHint> key_hints;
    // More lines for the help (?), after the keys, e.g. what the caller's tags mean
    std::vector<std::string> help_lines;
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
    // Keys (bytes) that finish picking too, like Enter, so the caller can do something about the
    // selected node (see TreeKeyboardSelector::get_expected_key())
    std::string expect_keys;
    // The id of the node to start with selected, rather than the root; "" (or one that isn't in
    // the tree): the root
    std::string selected_id;
    // A file descriptor to read previews from while picking (see PreviewsReader): the lines to
    // show in a column beside the tree while a node is selected, each ending with
    // updates_delimiter too; -1: none, and no column
    int preview_fd = -1;
    // A file descriptor to write the selected node's path to (as printed, ending with
    // updates_delimiter), at the start and whenever the selection moves, so the caller can work
    // something out about it (e.g. for its preview); -1: none
    int selection_fd = -1;
    // Keys (bytes) that tell the caller of the selection and the picks, through notify_fd, without
    // finishing picking (see Picker::notify()); -1: none
    std::string notify_keys;
    int notify_fd = -1;
};

// The keys row's keys, in its order: treepicker's main ones (Space and Enter, as options says
// they're used), the caller's (one with a key of treepicker's says what it does instead), then
// search, help and quit
std::vector<KeyHint> key_hints(const PickerOptions& options);

} // namespace picker

#endif
