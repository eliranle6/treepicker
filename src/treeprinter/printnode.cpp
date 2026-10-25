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

// The vertical lines of the ancestors that have siblings after them, under the printed subtree
// root's children (depth 1), which hang from the root's line
static void print_ancestors_lines(std::ostream &out, int depth,
                                  const std::vector<bool>& has_next_sibling) {
    for (int depth_idx = 1; depth_idx < depth; ++depth_idx) {
        if (has_next_sibling[depth_idx])
            out << VERTICAL_TREE_LINE << "   ";
        else
            out << "    ";
    }
}

void print_node(
        std::ostream &out,
        const PrintedLine& line,
        const std::shared_ptr<Node> selection,
        std::shared_ptr<Node> printed_subtree_root,
        Tree& tree,
        const std::vector<bool>& has_next_sibling,
        int window_width
        ) {
    const PrintedLine& pn = line;

    // Print selection marker if node is selected node
    const bool is_selection = pn.node == selection;
    const bool is_picked = pn.node->is_picked;
    const bool is_partly_picked = pn.node->is_partly_picked;

    if (is_selection) {
        out << ">";
        out << guishell::Color(guishell::BLACK_ON_BLUE);
    } else {
        out << " ";
    }
    // Then a column marking picked nodes (the root's line has none: it shows the root in red)
    if (pn.node->identifier != printed_subtree_root->identifier) {
        out << (is_picked ? "X" : is_partly_picked ? "-" : " ");
    }

    // The tree's lines: the ancestors', then the node's own connector
    print_ancestors_lines(out, pn.depth, has_next_sibling);
    if (pn.node->identifier != printed_subtree_root->identifier) {
        out << (pn.is_last_sibling ? LAST_CHILD_CONNECTOR : MIDDLE_CHILD_CONNECTOR)
            << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE;
    }

    // If printing root, print breadcrumbs up to node before printed subtree root
    if (pn.node == printed_subtree_root) {
        if (is_picked and not is_selection) {
            out << guishell::Color(guishell::RED);
        } else if (is_partly_picked and not is_selection) {
            out << guishell::Color(guishell::MAGENTA);
        }
        // After the one-column selection marker
        _print_breadcrumbs(out, printed_subtree_root, tree,
                           window_width > 0 ? std::max(1, window_width - 1) : 0);
        if ((is_picked or is_partly_picked) and not is_selection) {
            out << guishell::Color(guishell::DEFAULT);
        }
    } else {
        // Print node text
        out << " ";

        // Create current node text
        auto tag = guishell::printable(pn.node->tag);
        if (pn.node->children.size() and not pn.are_children_printed) {
            tag += std::string(" (...)");
        }

        // Dirs look like dirs even when they're empty, if the tree knows they're dirs
        if (pn.node->children.size() or pn.node->kind == 'd') {
            auto color = is_selection ? guishell::SELECTED_DIRECTORY : guishell::DIRECTORY;
            if (is_picked) {
                color = is_selection ? guishell::RED_ON_BLUE : guishell::RED;
            } else if (is_partly_picked) {
                color = is_selection ? guishell::MAGENTA_ON_BLUE : guishell::MAGENTA;
            } else if (pn.node->is_search_match and not is_selection) {
                color = guishell::YELLOW;
            }
            out << guishell::Color(color) << guishell::Bold() <<
                tag << guishell::Unbold() << guishell::Color(guishell::DEFAULT);

        } else {
            const bool is_highlighted = pn.node->is_search_match and not is_selection;
            if (is_picked) {
                out << guishell::Color(is_selection ? guishell::RED_ON_BLUE : guishell::RED);
            } else if (is_selection) {
                out << guishell::Color(guishell::WHITE_ON_BLUE);
            } else if (is_highlighted) {
                out << guishell::Color(guishell::YELLOW);
            }
            out << tag;
            if ((is_picked or is_highlighted) and not is_selection) {
                out << guishell::Color(guishell::DEFAULT);
            }
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
        tags.push_back(guishell::printable(current->tag));
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

void print_scroll_indicator(std::ostream &out, std::size_t nr_hidden_items, int depth,
                            const std::vector<bool>& has_next_sibling,
                            bool is_after_printed_siblings) {
    // Where a node has its selection marker and picked columns
    out << "  ";
    print_ancestors_lines(out, depth, has_next_sibling);
    out << (is_after_printed_siblings ? LAST_CHILD_CONNECTOR : MIDDLE_CHILD_CONNECTOR)
        << HORIZONTAL_TREE_LINE << HORIZONTAL_TREE_LINE
        << " ... " << nr_hidden_items << " more" << std::endl;
}

}
