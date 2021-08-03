#include "picker/tree_selector.h"

namespace picker {

TreeSelector::TreeSelector(treelib::Tree& tree, treelib::TreePrinter& tree_printer) :
    _tree(tree),
    _tree_printer(tree_printer),
    _selection(tree.get_root())
{
}

typename TreeSelector::NodePtr TreeSelector::get_selection(void) {
    return _selection;
}

void TreeSelector::explore_children_of_selection(void) {
    if (_selection == nullptr) {
    } else if (_selection->children.size() == 0) {
    } else {
        _selection = _selection->children[0];
        if ((not (_selection)->is_matching_search
            and not (_selection)->is_ancestor_of_matching_search)) {
            _advance_selection_at_same_tree_level(1);
        }
    }
}

void TreeSelector::move_one_up(void) {
    if (nullptr == _selection) {
        return;
    }
    auto parent = _tree.get_node(_selection->parent);
    _selection = parent;
}

void TreeSelector::move_to_next(void) {
    auto next_node = _tree_printer.get_next_printed_node_after_selected();

    if (next_node == nullptr) {
        // There can be few cases for next node being null:
        // 1. Printed tree is empty - do nothing
        // 2. Selection isn't visible but root is - select root
        // 3. Selected node is the selected node is the last printed node - do nothing
        const bool selection_isnt_visible_but_root_is =  (
                not _tree_printer.was_selection_printed()
                and _tree_printer.get_last_printed_node() != nullptr);
        if (selection_isnt_visible_but_root_is) {
            next_node = _tree_printer.get_printed_subtree_root();
        }
    }

    if (next_node != nullptr) {
        _selection = next_node;
    }
}

void TreeSelector::move_to_prev(void) {
    auto prev_node = _tree_printer.get_printed_node_before_selected();

    // Avoid jumping to parent on moving to 'previously printed',
    // (use 'move_one_up' instead)
    if (prev_node == _tree_printer.get_printed_subtree_root()
            and prev_node != _tree.get_root()) {
        return;
    }

    if (prev_node != nullptr) {
        _selection = prev_node;
    }
}

void TreeSelector::_advance_selection_at_same_tree_level(int difference) {
    if (nullptr == _selection.get()) {
        return;
    }
    if (_tree.get_root() == _selection) {
        return;
    }
    NodePtr parent = _tree.get_node(_selection->parent);
    if (parent.get() == nullptr) {
        return;
    }
    auto& children = parent->children;
    auto position = children.begin();
    // Find the index of the current selection in the array of paren'ts children
    // TODO: optimize this (to something better than O(n))
    for (; position != children.end() &&
        _selection->identifier != (*position)->identifier; ++position);
    assert(position != children.end());

    // Advance
    int direction = 0;
    if (difference > 0) {
        direction = 1;
    } else if (difference < 0) {
        direction = -1;
    }
    int nr_steps_to_take = abs(difference);
    int nr_steps_taken = 0;
    auto target = position;
    while (nr_steps_taken < nr_steps_to_take) {
        target += direction;
        if (target == children.end()) {
            break;
        } else if (position == children.begin() and direction == -1) {
            break;
        } else if ((*target)->is_matching_search or (*target)->is_ancestor_of_matching_search) {
            position = target;
            ++nr_steps_taken;
        }
    }

    // Set selection
    _selection = *position;
}

}
