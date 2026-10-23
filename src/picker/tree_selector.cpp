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
        return;
    }
    // Select the first child that an ongoing search doesn't hide, if there is one
    for (auto& child : _selection->children) {
        if (child->is_search_visible()) {
            _selection = child;
            return;
        }
    }
}

void TreeSelector::move_one_up(void) {
    if (nullptr == _selection) {
        return;
    }
    if (_tree.get_root() == _selection) {
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

void TreeSelector::select_visible_ancestor(void) {
    if (_selection == nullptr) {
        return;
    }
    // The root is always shown
    while (_selection != _tree.get_root() and not _selection->is_search_visible()) {
        _selection = _tree.get_node(_selection->parent);
    }
}

}
