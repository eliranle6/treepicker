#include <memory>
#include <iostream>
#include <string>

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
        TreeAnalysisInfo& info,
        Tree& tree,
		bool* depth_to_next_sibling
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
        const auto is_last_child = info.m_node_to_next_sibling_existence[pn.node->identifier];
        if (is_last_child)
            out << MIDDLE_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
        else
            out << LAST_CHILD_CONNECTOR << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
    }

    // If printing root, print breadcrumbs up to node before printed subtree root
    if (pn.node == printed_subtree_root) {
        _print_breadcrumbs(out, printed_subtree_root, tree);
    } else {
        // Print node text
        out << " ";

        // Create current node text
        auto tag = pn.node->tag;
        if (pn.node->children.size()) {
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
        Tree& tree
        ) {
    std::string breadcrumbs;
    std::string node_id = node->identifier;

    int counter = 0;
    while (++counter <= 22) {
        auto node = tree.get_node(node_id);
        breadcrumbs = node->tag + std::string(" > ") + breadcrumbs;
        node_id = node->parent;
        if (node == tree.get_root()) {
            break;
        }
    }

    out << breadcrumbs << std::endl;
}

}
