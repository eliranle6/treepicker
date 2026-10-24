#include <algorithm>
#include "picker/tree_selector.h"

namespace picker {

TreeSelector::TreeSelector(treelib::Tree& tree, treelib::TreePrinter& tree_printer) :
    _tree(tree),
    _tree_printer(tree_printer),
    _selection(tree.get_root()),
    _nr_picked(0),
    _nr_picked_files(0)
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

void TreeSelector::move_to_first_sibling(void) {
    if (_selection == nullptr or _selection == _tree.get_root()) {
        return;
    }
    for (auto& sibling : _tree.get_node(_selection->parent)->children) {
        if (sibling->is_search_visible()) {
            _selection = sibling;
            return;
        }
    }
}

std::pair<std::size_t, std::size_t> TreeSelector::get_place_among_siblings(bool filter) {
    if (_selection == nullptr or _selection == _tree.get_root()) {
        return {0, 0};
    }
    std::size_t place = 0, nr_siblings = 0;
    for (auto& sibling : _tree.get_node(_selection->parent)->children) {
        if (filter and not sibling->is_search_visible()) {
            continue;
        }
        ++nr_siblings;
        if (sibling == _selection) {
            place = nr_siblings;
        }
    }
    return {place, nr_siblings};
}

void TreeSelector::move_to_last_sibling(void) {
    if (_selection == nullptr or _selection == _tree.get_root()) {
        return;
    }
    auto& siblings = _tree.get_node(_selection->parent)->children;
    for (auto sibling = siblings.rbegin(); sibling != siblings.rend(); ++sibling) {
        if ((*sibling)->is_search_visible()) {
            _selection = *sibling;
            return;
        }
    }
}

// Moving goes through what's printed, line by line, so the screen stays put rather than being
// redrawn around each new selection. At the screen's edges it goes on, like scrolling a list: to the
// node that a bigger screen would show next (or before), and the printer shows it, moving its view
// as little as it can.

void TreeSelector::move_to_next(void) {
    auto next_node = _tree_printer.get_next_printed_node_after_selected();

    if (next_node == nullptr) {
        // There can be few cases for next node being null:
        // 1. Printed tree is empty - do nothing
        // 2. Selection isn't visible but root is - select root
        // 3. Selected node is the last printed node - the node after it, past the screen's edge
        const bool selection_isnt_visible_but_root_is =  (
                not _tree_printer.was_selection_printed()
                and _tree_printer.get_last_printed_node() != nullptr);
        if (selection_isnt_visible_but_root_is) {
            next_node = _tree_printer.get_printed_subtree_root();
        } else if (_selection != nullptr) {
            next_node = get_next_beyond_screen(_selection);
        }
    }

    if (next_node != nullptr) {
        _selection = next_node;
    }
}

TreeSelector::NodePtr TreeSelector::get_next_beyond_screen(NodePtr node) {
    // Not into the node's own children: they weren't printed (the node was the last line). So its
    // next sibling, or else the next sibling of the closest ancestor that has one.
    for (; node != _tree.get_root(); node = _tree.get_node(node->parent)) {
        auto& siblings = _tree.get_node(node->parent)->children;
        auto position = std::find(siblings.begin(), siblings.end(), node);
        for (auto sibling = position + 1; sibling < siblings.end(); ++sibling) {
            if ((*sibling)->is_search_visible()) {
                return *sibling;
            }
        }
    }
    return nullptr;
}

void TreeSelector::move_to_prev(void) {
    // Above the first child of the printed subtree's root, there's the root itself (shown as the
    // breadcrumbs line): past the screen's top edge, so selecting it scrolls the view up
    auto prev_node = _tree_printer.get_printed_node_before_selected();
    if (prev_node != nullptr) {
        _selection = prev_node;
    }
}

void TreeSelector::toggle_pick_of_selection(void) {
    if (_selection == nullptr) {
        return;
    }
    // Everything under it, whether an ongoing search shows it or not
    const bool is_picked = not _selection->is_picked;
    std::vector<treelib::Node*> dfs_stack = {_selection.get()};
    while (not dfs_stack.empty()) {
        auto node = dfs_stack.back();
        dfs_stack.pop_back();
        set_picked_flag(node, is_picked);
        node->is_partly_picked = false;
        for (auto& child : node->children) {
            dfs_stack.push_back(child.get());
        }
    }

    // Up from it, each node's state depends on its children's
    for (auto node = _selection; node != _tree.get_root();) {
        node = _tree.get_node(node->parent);
        update_from_children(node.get());
    }
}

void TreeSelector::update_from_children(treelib::Node* node) {
    if (node->children.empty()) {
        return;
    }
    bool are_all_picked = true;
    bool is_any_picked = false;
    for (auto& child : node->children) {
        are_all_picked = are_all_picked and child->is_picked;
        is_any_picked = is_any_picked or child->is_picked or child->is_partly_picked;
    }
    set_picked_flag(node, are_all_picked);
    node->is_partly_picked = not are_all_picked and is_any_picked;
}

void TreeSelector::update_picked_dirs(void) {
    if (_tree.get_root() == nullptr) {
        return;
    }
    // Children before their parents: a DFS's preorder, reversed
    std::vector<treelib::Node*> preorder;
    std::vector<treelib::Node*> dfs_stack = {_tree.get_root().get()};
    while (not dfs_stack.empty()) {
        auto node = dfs_stack.back();
        dfs_stack.pop_back();
        preorder.push_back(node);
        for (auto& child : node->children) {
            dfs_stack.push_back(child.get());
        }
    }
    for (auto node = preorder.rbegin(); node != preorder.rend(); ++node) {
        update_from_children(*node);
    }
}

void TreeSelector::set_picked(NodePtr node, bool is_picked) {
    if (node != nullptr) {
        set_picked_flag(node.get(), is_picked);
    }
}

void TreeSelector::set_picked_flag(treelib::Node* node, bool is_picked) {
    if (node->is_picked == is_picked) {
        return;
    }
    node->is_picked = is_picked;
    const int change = is_picked ? 1 : -1;
    _nr_picked += change;
    if (node->kind == 'f') {
        _nr_picked_files += change;
    }
}

std::vector<TreeSelector::NodePtr> TreeSelector::get_picked_nodes(void) {
    std::vector<NodePtr> picked;
    if (_tree.get_root() == nullptr) {
        return picked;
    }
    // Pointers to the nodes' shared_ptrs, to copy only the picked ones
    std::vector<const NodePtr*> dfs_stack = {&_tree.get_root()};
    while (not dfs_stack.empty() and picked.size() < _nr_picked) {
        const NodePtr& node = *dfs_stack.back();
        dfs_stack.pop_back();
        if (node->is_picked) {
            picked.push_back(node);
        }
        // Reversed, so the first child is popped first
        for (auto child = node->children.rbegin(); child != node->children.rend(); ++child) {
            dfs_stack.push_back(&*child);
        }
    }
    return picked;
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
