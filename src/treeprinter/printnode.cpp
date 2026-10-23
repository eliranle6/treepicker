#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "tree/tree.h"
#include "utils/guishell.h"
#include "printnode.h"

// --
//
#define HORIZONTAL_TREE_LINE ("\xe2\x94\x80")
// |
// |
// |
#define VERTICAL_TREE_LINE ("\xe2\x94\x82")
// |
// |--
// |
#define MIDDLE_CHILD_CONNECTOR ("\xe2\x94\x9c")
// |
// |--
//
#define LAST_CHILD_CONNECTOR  ("\xe2\x94\x94")


namespace treelib {

void print_node(
        std::ostream &out,
        PrintedNode& printed_node,
        const std::shared_ptr<Node> selection,
        std::shared_ptr<Node> printed_subtree_root,
        Tree& tree,
        bool* depth_to_next_sibling,
        bool is_last_in_pagination,
        int window_width
        ) {
    assert(depth_to_next_sibling != nullptr);
    PrintedNode& pn = printed_node;

    // Before printing, maintain the array of depth to -
    // [whether a next (=not printed yet) sibling exists]
    // in order to know whether to print connecting lines to next sibling of that
    // depth (node depth corresponds to indentation level), if siblint such exists
    //update_depth_to_next_sibling_map(pn.previous_depth, pn.depth, pn.previous_node);

    // Print selection marker if node is selected node
    const bool is_selection = pn.node == selection;

    if (is_selection) {
        out << ">";
        out << guishell::Color(guishell::BLACK_ON_BLUE);
    } else {
        out << " ";
    }
    if (pn.node->identifier != printed_subtree_root->identifier) {
        out << " ";
    }

    // Print connecting lines for each depth (node depth is manifested by the indentation
    // level
    for (int depth_idx = 1; depth_idx < pn.depth; ++depth_idx) {
        if (depth_to_next_sibling[depth_idx])
            out << VERTICAL_TREE_LINE << "   ";
        else
            out << "    ";
    }
    // Connecting lines special case - if node is last node in level
    if (pn.node->identifier != printed_subtree_root->identifier) {
        const auto is_last_child = depth_to_next_sibling[pn.depth];
        if (is_last_child or is_last_in_pagination)
            out << MIDDLE_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
        else
            out << LAST_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
    }

    // If printing root, print breadcrumbs up to node before printed subtree root
    if (pn.node == printed_subtree_root) {
        // After the one-column selection marker
        _print_breadcrumbs(out, printed_subtree_root, tree,
                           window_width > 0 ? std::max(1, window_width - 1) : 0);
    } else {
        // Print node text
        out << " ";

        // Create current node text
        auto tag = pn.node->tag;
        if (pn.node->children.size() and not pn.are_children_printed) {
            tag += std::string(" (...)");
        }

        if (pn.node->children.size()) {
            auto color = is_selection ? guishell::SELECTED_DIRECTORY :
                guishell::NON_SELECTED_DIRECTORY;
            out << guishell::Color(color) << guishell::Bold() <<
                tag << guishell::Unbold() << guishell::Color(guishell::DEFAULT);

        } else {
            if (is_selection) {
                out << guishell::Color(guishell::WHITE_ON_BLUE);
            }
            out << tag;
        }
        out << std::endl;
    }

    if (is_selection) {
        out << guishell::Color(guishell::DEFAULT);
    }
}

void _print_breadcrumbs(
        std::ostream &out,
        const std::shared_ptr<Node> node,
        Tree& tree,
        int max_width
        ) {
    static const std::string SEPARATOR = " > ";
    static const std::string ELLIPSIS = "...";

    // Tags from the node up to the root
    std::vector<std::string> tags;
    std::string node_id = node->identifier;
    while (true) {
        auto current = tree.get_node(node_id);
        tags.push_back(current->tag);
        if (current == tree.get_root()) {
            break;
        }
        node_id = current->parent;
    }

    // Join from the root down, leaving out the tags closest to the root while it's too wide.
    // The nearest dirs say more about where the selection is.
    std::size_t nr_tags = tags.size();
    auto join = [&](std::size_t nr_tags_to_join) {
        std::string breadcrumbs = nr_tags_to_join < tags.size() ? ELLIPSIS + SEPARATOR : "";
        for (std::size_t i = nr_tags_to_join; i > 0; --i) {
            breadcrumbs += tags[i - 1] + SEPARATOR;
        }
        return breadcrumbs;
    };
    std::string breadcrumbs = join(nr_tags);
    while (max_width > 0 and nr_tags > 1 and guishell::display_width(breadcrumbs) > max_width) {
        breadcrumbs = join(--nr_tags);
    }

    out << breadcrumbs << std::endl;
}

void print_top_scroll_indicator(std::ostream &out, std::size_t nr_hidden_items) {
    out << "  " << VERTICAL_TREE_LINE << "(" << nr_hidden_items << " more ...)" << std::endl;
}

}
