#ifndef PICKER_OPTIONS_H__
#define PICKER_OPTIONS_H__

#include <string>
#include <vector>

#include "tree/tree.h"

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

// How a state is shown for a group's nodes (see StateOption)
struct GroupMark {
    int group;
    std::string mark;
    std::string color;
};

// A state a node can be in (see PickerOptions::states): its name (printed with the node), its mark
// (one column, shown before the node) and color (a guishell::text_style() name), the key that
// sets it (0: none), and other marks for some groups' nodes (e.g. "copy here" is a different
// thing for nodes of each side)
struct StateOption {
    std::string name;
    std::string mark;
    std::string color;
    int key = 0;
    std::vector<GroupMark> group_marks;
};

// A tab (see PickerOptions::tabs): its title, and the groups of nodes it shows (indexes into
// PickerOptions::groups; empty: all the nodes)
struct TabOption {
    std::string title;
    std::vector<int> groups;
};

// A node's states (see PickerOptions::states): the ones it allows (a bit each, by their index), the
// one it starts in, and its group (-1: none)
struct NodeStates {
    std::string id;
    unsigned allowed = ~0u;
    int initial = 0;
    int group = -1;
};

struct PickerOptions {
    // Pick several nodes: Space picks or unpicks, Enter finishes
    bool is_multi = false;
    // With is_multi, Enter with nothing picked picks the selected node (and what's under it, as
    // Space does), rather than doing nothing
    bool is_selection_picked_if_none = false;
    // Lines shown above the tree
    std::string header;
    // Lines shown below the tree, above the bottom lines (each plain, or in styled parts, as
    // --preview's lines)
    std::string footer;
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
    // Keys that do so when pressed twice in a row, as vim's cc (the key is the expected one)
    std::string expect_twice_keys;
    // The id of the node to start with selected, rather than the root; "" (or one that isn't in
    // the tree): the root
    std::string selected_id;
    // A file descriptor to read previews from while picking (see PreviewsReader): the lines to
    // show in a column beside the tree while a node is selected, each ending with
    // updates_delimiter too; -1: none, and no column
    int preview_fd = -1;
    // Show the selected node's preview below the tree, above the stats line (in colored parts:
    // see parse_segments()), rather than in a column on the right, e.g. a menu's, saying
    // what its selected option would do
    bool is_preview_below = false;
    // A file descriptor to write the selected node's path to (as printed, ending with
    // updates_delimiter), at the start and whenever the selection moves, so the caller can work
    // something out about it (e.g. for its preview); -1: none
    int selection_fd = -1;
    // Keys (bytes) that tell the caller of the selection and the picks, through notify_fd, without
    // finishing picking (see Picker::notify()); -1: none
    std::string notify_keys;
    int notify_fd = -1;
    // Another tree, shown in the upper part of the right column (above the preview), to look
    // through beside the picked one, e.g. where they'd go: Ctrl-L moves in it, Ctrl-H back. Not
    // owned; nullptr: none.
    treelib::Tree* side_tree = nullptr;
    // A file descriptor to read the paths of the side tree's nodes to select from while picking
    // (ending with updates_delimiter), e.g. where the selected node's would go, unless the user is
    // moving in it; -1: none
    int side_select_fd = -1;
    // Start moving in the side tree, on the node at this path (or its closest ancestor there; "."
    // is its root); empty: start in the tree
    std::string side_start_path;
    // The key printed, as an expected key's, when Enter is pressed on a node of the side tree, which
    // finishes picking (see Picker::get_side_selection()); 0: Enter does nothing there
    int side_key = 0;
    // Keys that do so too, when pressed in the side tree (see get_side_pressed_key())
    std::string side_expect_keys;
    // The keys row while moving in the side tree: what its keys do there
    std::vector<KeyHint> side_key_hints;
    // What the keys row says of keys while nothing's picked, in place of what key_hints says
    std::vector<KeyHint> none_picked_key_hints;
    // States for the nodes, rather than picked or not (with is_multi): each node is in one, the
    // first (e.g. skip) unpicked, the others picked; Space cycles the selection's through the ones
    // it allows (a dir's: the ones all its nodes allow, set on all of them), and a state's key sets
    // it. Empty: no states.
    std::vector<StateOption> states;
    // The nodes' states (each a node without children; the others allow all and start in the
    // first), and their groups' names
    std::vector<NodeStates> node_states;
    std::vector<std::string> groups;
    // Tabs, a row of them above the tree, each showing some of the groups: Tab and Shift-Tab
    // switch. Empty: none.
    std::vector<TabOption> tabs;
};

// The keys row's keys, in its order: treepicker's main ones (Space and Enter, as options says
// they're used), the caller's (one with a key of treepicker's says what it does instead), then
// search, help and quit. With is_none_picked, what none_picked_key_hints says instead. With
// is_side_focused, the side tree's: side_key_hints, the way back, help and quit.
std::vector<KeyHint> key_hints(const PickerOptions& options, bool is_none_picked = false,
                               bool is_side_focused = false);

} // namespace picker

#endif
