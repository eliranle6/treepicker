#include <cassert>
#include "treeprinter.h"
#include "paginator.h"
#include "printnode.h"
#include "treewindowfitter.h"

namespace treelib {

TreePrinter::TreePrinter(Tree& tree): _tree(tree),
                                      _printed_subtree_root(tree.get_root()),
                                      _filter_search_nodes(false),
                                      _was_selection_printed(false)
{}

bool TreePrinter::is_shown(const std::shared_ptr<Node>& node) {
    // The root is shown even when nothing matches, so the screen is never empty
    return not _filter_search_nodes or node == _tree.get_root() or node->is_search_visible;
}

std::vector<const std::shared_ptr<Node>*> TreePrinter::get_shown_children(
        const std::shared_ptr<Node>& node) {
    std::vector<const std::shared_ptr<Node>*> children;
    // Reserved, since growing the vector step by step took 1-2ms for 100k children
    children.reserve(node->children.size());
    for (auto& child : node->children) {
        if (is_shown(child)) {
            children.push_back(&child);
        }
    }
    return children;
}

int TreePrinter::print(std::ostream &out,
                       bool filter_search_nodes,
                       std::shared_ptr<Node> selection,
                       int window_height,
                       int window_width) {
    layout(filter_search_nodes, selection, window_height);

    // Whether the line last drawn at each depth has siblings after it, so the lines under it
    // continue its parent's vertical line
    std::vector<bool> has_next_sibling;
    for (auto& line : _lines) {
        has_next_sibling.resize(std::max<std::size_t>(has_next_sibling.size(), line.depth + 1));
        has_next_sibling[line.depth] = not line.is_last_sibling;
        if (line.node == nullptr) {
            print_scroll_indicator(out, line.nr_hidden, line.depth, has_next_sibling,
                                   line.is_last_sibling);
        } else {
            print_node(out, line, selection, _printed_subtree_root, _tree, has_next_sibling,
                       window_width);
        }
    }
    return _lines.size();
}

void TreePrinter::layout(bool filter_search_nodes, std::shared_ptr<Node> selection,
                         int window_height) {
    assert(selection);
    _lines.clear();
    _printed_node_before_selected.reset();
    _next_printed_node_after_selected.reset();
    _was_selection_printed = false;
    if (nullptr == _tree.get_root()) {
        return;
    }
    _filter_search_nodes = filter_search_nodes;

    int nr_levels;
    std::tie(_printed_subtree_root, std::ignore, nr_levels) = choose_printed_subtree_root(
        _tree, selection, _tree.get_node_depth(selection), window_height, _printed_subtree_root);

    // The root and the levels under it that fit in the window. If not even its children all fit,
    // the ones around the selection: choose_printed_subtree_root() keeps the selection at most
    // one level under the root then, so only the root's children are ever paginated.
    _lines.push_back(PrintedLine{_printed_subtree_root, 0, true, nr_levels > 1, 0});
    if (nr_levels > 1) {
        add_descendants(_printed_subtree_root, 1, nr_levels - 1);
    } else {
        add_paginated_children(_printed_subtree_root, selection, window_height);
    }

    // The nodes around the selection, for moving through what's shown
    std::shared_ptr<Node> previous;
    for (auto& line : _lines) {
        if (line.node == nullptr) {
            continue;
        }
        if (_was_selection_printed) {
            _next_printed_node_after_selected = line.node;
            break;
        }
        if (line.node == selection) {
            _was_selection_printed = true;
            _printed_node_before_selected = previous;
        }
        previous = line.node;
    }
}

void TreePrinter::add_descendants(const std::shared_ptr<Node>& node, int depth, int max_depth) {
    const auto children = get_shown_children(node);
    for (std::size_t i = 0; i < children.size(); ++i) {
        const auto& child = *children[i];
        _lines.push_back(PrintedLine{child, depth, i == children.size() - 1, depth < max_depth, 0});
        if (depth < max_depth) {
            add_descendants(child, depth + 1, max_depth);
        }
    }
}

void TreePrinter::add_paginated_children(const std::shared_ptr<Node>& root,
                                         std::shared_ptr<Node> selection, int window_height) {
    const auto children = get_shown_children(root);
    std::size_t selection_index = 0;
    while (selection_index < children.size() and *children[selection_index] != selection) {
        ++selection_index;
    }
    if (selection_index == children.size()) {
        selection_index = 0;
    }

    // The root's line takes a row
    int nr_hidden_before, nr_hidden_after;
    std::tie(nr_hidden_before, nr_hidden_after) =
        paginate(children.size(), window_height - 1, selection_index);
    if (nr_hidden_before > 0) {
        _lines.push_back(PrintedLine{nullptr, 1, false, false,
                                     static_cast<std::size_t>(nr_hidden_before)});
    }
    const std::size_t end = children.size() - nr_hidden_after;
    for (std::size_t i = nr_hidden_before; i < end; ++i) {
        const bool is_last = i == end - 1 and nr_hidden_after == 0;
        _lines.push_back(PrintedLine{*children[i], 1, is_last, false, 0});
    }
    if (nr_hidden_after > 0) {
        _lines.push_back(PrintedLine{nullptr, 1, true, false,
                                     static_cast<std::size_t>(nr_hidden_after)});
    }
}

std::shared_ptr<treelib::Node> TreePrinter::get_next_printed_node_after_selected(void) {
    return _next_printed_node_after_selected;
}

std::shared_ptr<treelib::Node> TreePrinter::get_printed_node_before_selected(void) {
    return _printed_node_before_selected;
}

bool TreePrinter::was_selection_printed(void) {
    return _was_selection_printed;
}

std::shared_ptr<treelib::Node> TreePrinter::get_first_printed_node(void) {
    return _lines.empty() ? nullptr : _lines.front().node;
}

std::shared_ptr<treelib::Node> TreePrinter::get_last_printed_node(void) {
    for (auto line = _lines.rbegin(); line != _lines.rend(); ++line) {
        if (line->node != nullptr) {
            return line->node;
        }
    }
    return nullptr;
}

std::shared_ptr<treelib::Node> TreePrinter::get_printed_subtree_root(void) {
    return _printed_subtree_root;
}

}
