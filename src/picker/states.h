#ifndef PICKER_STATES_H__
#define PICKER_STATES_H__

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "picker/picker_options.h"
#include "picker/tree_selector.h"
#include "tree/tree.h"
#include "treeprinter/printnode.h"

namespace picker {

// The nodes' states, when the caller gives them (PickerOptions::states), rather than picked or
// not: each node without children is in one of them, and a dir is in its nodes' state if they're
// all in the same one, else Node::MIXED_STATE. A node is picked when it isn't in the first state
// (e.g. skip), so the selector's picked counts and dirs follow.
class States
{
public:
    using NodePtr = std::shared_ptr<treelib::Node>;

    // Put the nodes in their states (options.node_states; the others in the first)
    States(treelib::Tree& tree, TreeSelector& selector, const PickerOptions& options);

    bool is_on(void) const { return not _states.empty(); }

    // How the printer shows them
    const treelib::StateMarks* get_marks(void) const { return is_on() ? &_marks : nullptr; }

    // Put node, and with only_shown what an ongoing search shows under it (else all under it), in
    // state: the nodes that allow it
    void set(const NodePtr& node, int state, bool only_shown);
    // Put them in the next state they all allow, after node's (from the first, if they differ)
    void cycle(const NodePtr& node, bool only_shown);
    // Put all the nodes in the first state
    void reset(void);

    // The state the key sets, or -1 if none does
    int get_state_of_key(int key) const;
    // The state of that name, or -1 if there's none
    int get_state_named(const std::string& name) const;

    // How many nodes are in each state but the first, by their marks: e.g. "+ 120  ✗ 30  → 2"
    std::string get_counts_text(void) const;

    // The nodes not in the first state, in the tree's order, each with its state's name
    std::vector<std::pair<std::string, NodePtr>> get_set_nodes(void);

private:
    treelib::Tree& _tree;
    TreeSelector& _selector;
    std::vector<StateOption> _states;
    treelib::StateMarks _marks;
    // How many nodes without children are in each state, and its group: [state][group + 1]
    std::vector<std::vector<std::size_t>> _counts;

    // Put a node without children in state
    void set_one(treelib::Node* node, int state);
    // Update a dir's state, and picked state, from its children's
    void update_dir(treelib::Node* node);
    // Update the dirs under node (children first), then the ones above it
    void update_around(const NodePtr& node);
    // The nodes without children under node (node itself, if it has none), only the shown ones
    // with only_shown
    std::vector<treelib::Node*> get_files_under(const NodePtr& node, bool only_shown);
    // The mark a state shows for a group's nodes
    const treelib::StateMark& get_mark(int state, int group) const;
};

} // namespace picker

#endif
