#include "gtest/gtest.h"

#include "picker/tree_selector.h"
#include "tree/tests/common.h"
#include "tree/search.h"
#include <random>


TEST(tree_selector, default_selection) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    ASSERT_EQ(tree_selector.get_selection(), tree.get_root());
}

TEST(tree_selector, default_selection_in_empty_tree) {
    treelib::Tree tree;
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    ASSERT_EQ(tree_selector.get_selection(), nullptr);
}

TEST(tree_selector, explore_selection) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");
}

TEST(tree_selector, explore_selection_in_empty_tree) {
    treelib::Tree tree;
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection(), nullptr);
}

TEST(tree_selector, move_one_next) {
    // Setup
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    std::ostringstream out;
    picker::TreeSelector tree_selector(tree, tree_printer);
    ASSERT_EQ(tree_selector.get_selection()->identifier, "people");
    tree_printer.scan_nodes_to_print(out, true, tree.get_root(), 100);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");

    // Run
    tree_printer.scan_nodes_to_print(std::cout, true, tree_selector.get_selection(), 100);
    tree_selector.move_to_next();

    // Validate
    ASSERT_EQ(tree_selector.get_selection()->identifier, "poor-dumb");
}

TEST(tree_selector, move_one_prev) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    tree_printer.scan_nodes_to_print(std::cout, true, tree_selector.get_selection(), 100);
    tree_selector.move_to_next();
    tree_printer.scan_nodes_to_print(std::cout, true, tree_selector.get_selection(), 100);
    tree_selector.move_to_prev();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");
}

TEST(tree_selector, move_one_prev__does_not_pass_bounds) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    tree_selector.move_to_prev();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");
}

TEST(tree_selector, move_one_up) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    tree_selector.move_one_up();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "people");
}

TEST(tree_selector, move_one_up_does_not_pass_bounds) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    tree_selector.move_one_up();
    tree_selector.move_one_up();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "people");
}

TEST(tree_selector, explore_selection_skips_non_maching_children) {
    auto tree = get_simple_tree();
    tree.get_node("dumb")->is_matching_search = false;
    tree.get_node("dumb")->is_ancestor_of_matching_search = false;
        tree.get_node("rich-dumb")->is_matching_search = false;
        tree.get_node("rich-dumb")->is_ancestor_of_matching_search = false;
        tree.get_node("poor-dumb")->is_matching_search = false;
        tree.get_node("poor-dumb")->is_ancestor_of_matching_search = false;

    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "smart");
}

TEST(tree_selector, next_skips_non_matching_children) {
    // Setup
    auto tree = get_simple_tree();
    // Leave only root, dumb and empty
    tree.get_node("rich-dumb")->is_ancestor_of_matching_search = false;
    tree.get_node("poor-dumb")->is_ancestor_of_matching_search = false;
    tree.get_node("rich-dumb")->is_matching_search = false;
    tree.get_node("poor-dumb")->is_matching_search = false;
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    // Go to first subnode of root, 'dumb'
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");

    // Move next, skipping subnodes of dumb
    std::ostringstream out;
    tree_printer.scan_nodes_to_print(std::cout, true, tree_selector.get_selection(), 100);
    tree_selector.move_to_next();

    ASSERT_EQ(tree_selector.get_selection()->identifier, "smart");
}

// Print the tree the way the picker does before each move, since moving to the next/previous
// node relies on what was printed
static std::vector<std::string> print(treelib::TreePrinter& tree_printer,
                                      picker::TreeSelector& tree_selector,
                                      int window_height = 100, bool filter = false) {
    std::ostringstream out;
    tree_printer.print(out, filter, tree_selector.get_selection(), window_height);
    // Without escape codes, which may follow the last newline
    return split_string_by_newline(strip_escape_codes(out.str()));
}

TEST(tree_selector, explore_selection_without_visible_children_stays) {
    auto tree = get_simple_tree();
    treelib::Search(tree).search("smart");
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "smart");
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "poor-smart");
    // A leaf
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "poor-smart");
}

TEST(tree_selector, select_visible_ancestor) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "poor-dumb");

    // Still visible: stays
    treelib::Search(tree).search("Poor");
    tree_selector.select_visible_ancestor();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "poor-dumb");

    // Hidden, but its parent isn't
    treelib::Search(tree).search("Rich dumb");
    tree_selector.select_visible_ancestor();
    ASSERT_EQ(tree_selector.get_selection()->identifier, "dumb");

    // Nothing matches: the root
    treelib::Search(tree).search("no such node");
    tree_selector.select_visible_ancestor();
    ASSERT_EQ(tree_selector.get_selection(), tree.get_root());
}

TEST(tree_selector, move_to_next_at_the_last_node_stays) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    for (int i = 0; i < 20; ++i) {
        print(tree_printer, tree_selector);
        tree_selector.move_to_next();
    }
    ASSERT_EQ(tree_selector.get_selection()->identifier, "rich-smart");
}

TEST(tree_selector, walk_through_a_dir_bigger_than_the_window) {
    // Through the pagination, one child at a time, and back
    auto tree = get_flat_tree(60);
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    auto& children = tree.get_root()->children;
    for (std::size_t i = 1; i < children.size(); ++i) {
        print(tree_printer, tree_selector, 10);
        tree_selector.move_to_next();
        ASSERT_EQ(tree_selector.get_selection(), children[i]);
    }
    for (int i = children.size() - 2; i >= 0; --i) {
        print(tree_printer, tree_selector, 10);
        tree_selector.move_to_prev();
        ASSERT_EQ(tree_selector.get_selection(), children[i]);
    }
}

// A random tree of nr_nodes nodes, with tags made of a few letters so searches match some
static treelib::Tree get_random_tree(std::mt19937& random, int nr_nodes) {
    treelib::Tree tree;
    tree.create_node("root", "0", "", nullptr);
    for (int i = 1; i < nr_nodes; ++i) {
        std::string tag;
        for (int j = 0, length = 1 + random() % 4; j < length; ++j) {
            tag += "abcde"[random() % 5];
        }
        // Lean towards recent nodes as parents, for deeper trees
        int parent = std::max(0, i - 1 - static_cast<int>(random() % std::min(i, 8)));
        if (random() % 3 == 0) {
            parent = random() % i;
        }
        tree.create_node(tag + std::to_string(i), std::to_string(i), std::to_string(parent),
                         nullptr);
    }
    return tree;
}

TEST(tree_selector, random_walks_keep_the_selection_shown) {
    // Like the picker: random moves and searches, printing after each one. The selection must
    // always be a node that's shown, and the tree must fit in the window.
    for (int seed = 0; seed < 300; ++seed) {
        std::mt19937 random(seed);
        auto tree = get_random_tree(random, 1 + random() % 80);
        treelib::TreePrinter tree_printer(tree);
        picker::TreeSelector tree_selector(tree, tree_printer);
        treelib::Search search(tree);
        const int window_height = 4 + random() % 12;
        std::string keyword;
        std::string actions;

        for (int step = 0; step < 60; ++step) {
            switch (random() % 6) {
                case 0: tree_selector.move_to_next(); actions += 'j'; break;
                case 1: tree_selector.move_to_prev(); actions += 'k'; break;
                case 2: tree_selector.explore_children_of_selection(); actions += 'l'; break;
                case 3: tree_selector.move_one_up(); actions += 'h'; break;
                case 4:
                    if (random() % 3 == 0 or keyword.size() > 2) {
                        keyword.clear();
                    } else {
                        keyword += "abcde"[random() % 5];
                    }
                    search.search(keyword);
                    tree_selector.select_visible_ancestor();
                    actions += "/" + keyword + "/";
                    break;
                case 5: actions += '.'; break;  // Just print again
            }
            SCOPED_TRACE("seed=" + std::to_string(seed) + " window_height=" +
                         std::to_string(window_height) + " actions=" + actions);

            auto selection = tree_selector.get_selection();
            ASSERT_NE(selection, nullptr);
            ASSERT_TRUE(selection == tree.get_root() or selection->is_search_visible());
            auto lines = print(tree_printer, tree_selector, window_height, not keyword.empty());
            std::string screen;
            for (auto& line : lines) {
                screen += line + "\n";
            }
            ASSERT_TRUE(tree_printer.was_selection_printed()) << screen;
            ASSERT_LE(lines.size(), window_height) << screen;
        }
    }
}

TEST(tree_selector, walk_through_a_filtered_dir_bigger_than_the_window) {
    // Pagination once counted hidden children too, so moving next could select a hidden one
    auto tree = get_flat_tree(60);
    treelib::Search(tree).search("5");  // child000005, child000015, ... child000050..59
    std::vector<std::shared_ptr<treelib::Node>> visible_children;
    for (auto& child : tree.get_root()->children) {
        if (child->is_search_visible()) {
            visible_children.push_back(child);
        }
    }
    ASSERT_EQ(visible_children.size(), 15);

    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection(), visible_children[0]);
    for (std::size_t i = 1; i < visible_children.size(); ++i) {
        auto lines = print(tree_printer, tree_selector, 6, true);
        ASSERT_LE(lines.size(), 6);
        tree_selector.move_to_next();
        ASSERT_EQ(tree_selector.get_selection(), visible_children[i]);
    }
}

static std::vector<std::string> ids(const std::vector<std::shared_ptr<treelib::Node>>& nodes) {
    std::vector<std::string> result;
    for (auto& node : nodes) {
        result.push_back(node->identifier);
    }
    return result;
}

TEST(tree_selector, toggle_pick_picks_the_subtree) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();  // dumb

    tree_selector.toggle_pick_of_selection();

    ASSERT_EQ(tree_selector.get_nr_picked(), 3);
    ASSERT_EQ(ids(tree_selector.get_picked_nodes()),
              std::vector<std::string>({"dumb", "poor-dumb", "rich-dumb"}));
    ASSERT_FALSE(tree.get_node("smart")->is_picked);
}

TEST(tree_selector, toggle_pick_again_unpicks) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.toggle_pick_of_selection();
    ASSERT_EQ(tree_selector.get_nr_picked(), 7);

    tree_selector.toggle_pick_of_selection();

    ASSERT_EQ(tree_selector.get_nr_picked(), 0);
    ASSERT_TRUE(tree_selector.get_picked_nodes().empty());
}

TEST(tree_selector, unpick_part_of_a_picked_subtree) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();  // dumb
    tree_selector.toggle_pick_of_selection();
    tree_selector.explore_children_of_selection();  // poor-dumb

    tree_selector.toggle_pick_of_selection();

    // Not all under dumb is picked anymore, so dumb is only partly picked
    ASSERT_EQ(ids(tree_selector.get_picked_nodes()), std::vector<std::string>({"rich-dumb"}));
    ASSERT_EQ(tree_selector.get_nr_picked(), 1);
    ASSERT_FALSE(tree.get_node("dumb")->is_picked);
    ASSERT_TRUE(tree.get_node("dumb")->is_partly_picked);
}

TEST(tree_selector, pick_inside_a_picked_subtree_counts_once) {
    // Picking a node that's already picked (as part of its parent) unpicks it rather than
    // counting it twice
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();  // dumb
    tree_selector.explore_children_of_selection();  // poor-dumb
    tree_selector.toggle_pick_of_selection();
    tree_selector.move_one_up();  // dumb

    tree_selector.toggle_pick_of_selection();

    ASSERT_EQ(tree_selector.get_nr_picked(), 3);
    ASSERT_EQ(ids(tree_selector.get_picked_nodes()),
              std::vector<std::string>({"dumb", "poor-dumb", "rich-dumb"}));
}

TEST(tree_selector, toggle_pick_includes_nodes_hidden_by_a_search) {
    auto tree = get_simple_tree();
    treelib::Search(tree).search("rich");
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();  // dumb

    tree_selector.toggle_pick_of_selection();

    ASSERT_TRUE(tree.get_node("poor-dumb")->is_picked);
    ASSERT_EQ(tree_selector.get_nr_picked(), 3);
}

TEST(tree_selector, picked_nodes_are_in_the_tree_order) {
    // Whatever the order they were picked in
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();  // dumb
    print(tree_printer, tree_selector);
    tree_selector.move_to_next();  // poor-dumb
    print(tree_printer, tree_selector);
    tree_selector.move_to_next();  // rich-dumb
    tree_selector.toggle_pick_of_selection();
    print(tree_printer, tree_selector);
    tree_selector.move_to_next();  // smart
    print(tree_printer, tree_selector);
    tree_selector.move_to_next();  // poor-smart
    tree_selector.toggle_pick_of_selection();
    tree_selector.move_one_up();
    tree_selector.move_one_up();  // people
    tree_selector.explore_children_of_selection();  // dumb
    tree_selector.explore_children_of_selection();  // poor-dumb
    tree_selector.toggle_pick_of_selection();

    // Both of dumb's are picked, so dumb is too
    ASSERT_EQ(ids(tree_selector.get_picked_nodes()),
              std::vector<std::string>({"dumb", "poor-dumb", "rich-dumb", "poor-smart"}));
}

TEST(tree_selector, move_to_first_and_last_sibling) {
    auto tree = get_flat_tree(10);
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();

    tree_selector.move_to_last_sibling();
    ASSERT_EQ(tree_selector.get_selection()->tag, "child000009");
    tree_selector.move_to_first_sibling();
    ASSERT_EQ(tree_selector.get_selection()->tag, "child000000");
}

TEST(tree_selector, move_to_first_and_last_sibling_skips_hidden_ones) {
    auto tree = get_flat_tree(10);
    // As if a search hid the first and the last
    for (auto id : {"child000000", "child000009"}) {
        tree.get_node(id)->is_matching_search = false;
        tree.get_node(id)->is_ancestor_of_matching_search = false;
    }
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();
    ASSERT_EQ(tree_selector.get_selection()->tag, "child000001");

    tree_selector.move_to_last_sibling();
    ASSERT_EQ(tree_selector.get_selection()->tag, "child000008");
    tree_selector.move_to_first_sibling();
    ASSERT_EQ(tree_selector.get_selection()->tag, "child000001");
}

TEST(tree_selector, move_to_sibling_of_the_root_stays) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.move_to_last_sibling();
    ASSERT_EQ(tree_selector.get_selection(), tree.get_root());
}

TEST(tree_selector, set_picked_picks_the_node_alone) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);

    tree_selector.set_picked(tree.get_node("dumb"), true);
    tree_selector.set_picked(tree.get_node("dumb"), true);  // Counts once
    ASSERT_EQ(ids(tree_selector.get_picked_nodes()), std::vector<std::string>({"dumb"}));
    ASSERT_EQ(tree_selector.get_nr_picked(), 1);

    tree_selector.set_picked(tree.get_node("dumb"), false);
    tree_selector.set_picked(nullptr, true);  // E.g. an id that isn't in the tree
    ASSERT_EQ(tree_selector.get_nr_picked(), 0);
}


// In the simple tree each node with children has two: the first is reached with explore, the
// other with move_to_last_sibling

TEST(tree_selector, picking_all_of_a_dirs_children_picks_it) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();  // dumb
    tree_selector.explore_children_of_selection();  // poor-dumb
    tree_selector.toggle_pick_of_selection();
    ASSERT_FALSE(tree.get_node("dumb")->is_picked);
    ASSERT_TRUE(tree.get_node("dumb")->is_partly_picked);
    ASSERT_TRUE(tree.get_root()->is_partly_picked);

    tree_selector.move_to_last_sibling();  // rich-dumb
    tree_selector.toggle_pick_of_selection();

    ASSERT_TRUE(tree.get_node("dumb")->is_picked);
    ASSERT_FALSE(tree.get_node("dumb")->is_partly_picked);
    // Smart isn't picked, so the root is still only partly
    ASSERT_FALSE(tree.get_root()->is_picked);
    ASSERT_TRUE(tree.get_root()->is_partly_picked);
    ASSERT_EQ(tree_selector.get_nr_picked(), 3);
}

TEST(tree_selector, picking_everything_bit_by_bit_picks_the_root) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();  // dumb
    tree_selector.toggle_pick_of_selection();
    tree_selector.move_to_last_sibling();  // smart
    tree_selector.toggle_pick_of_selection();

    ASSERT_TRUE(tree.get_root()->is_picked);
    ASSERT_EQ(tree_selector.get_nr_picked(), 7);
}

TEST(tree_selector, unpicking_a_file_unpicks_its_ancestors) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.toggle_pick_of_selection();  // Everything
    tree_selector.explore_children_of_selection();  // dumb
    tree_selector.explore_children_of_selection();  // poor-dumb

    tree_selector.toggle_pick_of_selection();

    for (auto id : {"people", "dumb"}) {
        ASSERT_FALSE(tree.get_node(id)->is_picked) << id;
        ASSERT_TRUE(tree.get_node(id)->is_partly_picked) << id;
    }
    ASSERT_TRUE(tree.get_node("smart")->is_picked);
    ASSERT_EQ(ids(tree_selector.get_picked_nodes()),
              std::vector<std::string>({"rich-dumb", "smart", "poor-smart", "rich-smart"}));
}

TEST(tree_selector, picking_a_partly_picked_dir_picks_all_of_it) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();  // dumb
    tree_selector.explore_children_of_selection();  // poor-dumb
    tree_selector.toggle_pick_of_selection();
    tree_selector.move_one_up();  // dumb, partly picked

    tree_selector.toggle_pick_of_selection();

    ASSERT_TRUE(tree.get_node("dumb")->is_picked);
    ASSERT_TRUE(tree.get_node("rich-dumb")->is_picked);
}

TEST(tree_selector, unpicking_the_last_picked_file_clears_the_partly_picked) {
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    tree_selector.explore_children_of_selection();  // dumb
    tree_selector.explore_children_of_selection();  // poor-dumb
    tree_selector.toggle_pick_of_selection();
    tree_selector.toggle_pick_of_selection();

    for (auto id : {"people", "dumb"}) {
        ASSERT_FALSE(tree.get_node(id)->is_partly_picked) << id;
    }
    ASSERT_EQ(tree_selector.get_nr_picked(), 0);
}

TEST(tree_selector, update_picked_dirs) {
    // E.g. after starting with some nodes picked
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);
    picker::TreeSelector tree_selector(tree, tree_printer);
    for (auto id : {"poor-dumb", "rich-dumb", "poor-smart"}) {
        tree_selector.set_picked(tree.get_node(id), true);
    }

    tree_selector.update_picked_dirs();

    ASSERT_TRUE(tree.get_node("dumb")->is_picked);
    ASSERT_TRUE(tree.get_node("smart")->is_partly_picked);
    ASSERT_TRUE(tree.get_root()->is_partly_picked);
    ASSERT_EQ(tree_selector.get_nr_picked(), 4);
}
