#include <assert.h>
#include <sstream>
#include <iostream>
#include <gtest/gtest.h>

#include "tree/tree.h"
#include "tree/tests/common.h"
#include "treeprinter/treeprinter.h"
#include "tree/search.h"


static constexpr char SELECTED_ROW_FIRST_CHAR = '\x1B';


TEST(treeprinter, print_basic_tree) {
    // Setup
    // Store original cout buffer before mocking it
    std::ostringstream out;
    treelib::Tree tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);

    // Run
    auto selection = tree.get_root();
    tree_printer.print(out, true, selection, 100);

    // Validate
    auto tree_lines = split_string_by_newline(out.str());
    std::string actual = out.str();
    ASSERT_TRUE(does_string_contain_substring(tree_lines[0], "People"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[1], "Dumb"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[2], "Poor dumb"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[3], "Rich dumb"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[4], "Smart"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[5], "Poor smart"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[6], "Rich smart"));
}

TEST(treeprinter, test_different_selection) {
    // Choose 'dumb' intead of root should move the '>' char to the beginning of dumb
    //  People
    //> +-- Dumb people
    //  |   +-- Poor dumb people
    //  |   +-- Rich dumb people
    //  +-- Smart people
    //      +-- Poor smart people
    //      +-- Rich smart people

    // Setup
    // Store original cout buffer before mocking it
    std::ostringstream out;
    treelib::Tree tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);

    // Run
    // Select 'dumb' instead of root
    auto selection = tree.get_node("dumb");
    tree_printer.print(out, true, selection, 100);

    // Validate
    std::string actual = out.str();
    auto tree_lines = split_string_by_newline(out.str());
    auto dumb_line = tree_lines[1];

    ASSERT_EQ(dumb_line[1], SELECTED_ROW_FIRST_CHAR);
    ASSERT_NE(dumb_line[0], SELECTED_ROW_FIRST_CHAR);
}

TEST(treeprinter, printing_tree_smaller_than_window_omits_higher_levels) {
    // Printed the following tree with window:
    // -> People
    // +-- Dumb people
    // |   +-- Poor dumb people
    // |   +-- Rich dumb people
    // +-- Smart people
    //     +-- Poor smart people
    //     +-- Rich smart people
    //
    // with a window size of 3 will yield only these rows:
    // -> People
    // +-- Dumb people
    // +-- Smart people

    // Setup
    // Store original cout buffer before mocking it
    std::ostringstream out;
    constexpr static int window_height = 3;
    treelib::Tree tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);

    // Run
    tree_printer.print(out, true, tree.get_root(), window_height);

    // Validate
    auto tree_lines = split_string_by_newline(out.str());

    ASSERT_TRUE(does_string_contain_substring(tree_lines[0], "People"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[1], "Dumb people"));
    ASSERT_TRUE(does_string_contain_substring(tree_lines[2], "Smart people"));
}

TEST(treeprinter, get_next_printed_node) {
    // -> People
    // +-- Dumb people
    // |   +-- Poor dumb people
    // |   +-- Rich dumb people
    // +-- Smart people
    //     +-- Poor smart people
    //     +-- Rich smart people
    //
    // Selected is People, so next printed node should be 'Dumb people'

    treelib::Tree tree = get_simple_tree();
    std::ostringstream out;
    treelib::TreePrinter tree_printer(tree);
    constexpr static int window_height = 6;
    tree_printer.print(out, true, tree.get_root(), window_height);

    auto expected = tree.get_node("dumb");
    auto actual = tree_printer.get_next_printed_node_after_selected();

    ASSERT_EQ(expected, actual);
}

TEST(treeprinter, get_previously_printed_node) {
    treelib::Tree tree = get_simple_tree();
    std::ostringstream out;
    treelib::TreePrinter tree_printer(tree);
    constexpr static int window_height = 6;
    tree_printer.print(out, true, tree.get_node("poor-smart"), window_height);

    auto actual = tree_printer.get_printed_node_before_selected();
    auto expected = tree.get_node("smart");
    ASSERT_EQ(expected, actual);
}

static std::vector<std::string> print_lines(treelib::TreePrinter& tree_printer,
                                            std::shared_ptr<treelib::Node> selection,
                                            int window_height, bool filter = false) {
    std::ostringstream out;
    tree_printer.print(out, filter, selection, window_height);
    return split_string_by_newline(strip_escape_codes(out.str()));
}

TEST(treeprinter, tags_are_printed_as_is) {
    treelib::Tree tree;
    tree.create_node("root", "root", "");
    tree.create_node("100%_%s%n.txt", "file", "root");
    treelib::TreePrinter tree_printer(tree);

    auto lines = print_lines(tree_printer, tree.get_root(), 10);

    ASSERT_TRUE(does_string_contain_substring(lines[1], "100%_%s%n.txt"));
}

TEST(treeprinter, control_characters_in_tags_are_printed_as_question_marks) {
    treelib::Tree tree;
    tree.create_node("ro\tot", "root", "");
    tree.create_node("line\nbreak\x1b[31m", "file", "root");
    treelib::TreePrinter tree_printer(tree);

    auto lines = print_lines(tree_printer, tree.get_root(), 10);

    ASSERT_EQ(lines.size(), 2);
    ASSERT_TRUE(does_string_contain_substring(lines[0], "ro?ot"));
    ASSERT_TRUE(does_string_contain_substring(lines[1], "line?break?[31m"));
}

TEST(treeprinter, search_hides_nodes_that_dont_match) {
    auto tree = get_simple_tree();
    treelib::Search(tree).search("Rich smart");
    treelib::TreePrinter tree_printer(tree);

    auto lines = print_lines(tree_printer, tree.get_root(), 10, true);

    ASSERT_EQ(lines.size(), 3);
    ASSERT_TRUE(does_string_contain_substring(lines[0], "People"));
    ASSERT_TRUE(does_string_contain_substring(lines[1], "Smart people"));
    ASSERT_TRUE(does_string_contain_substring(lines[2], "Rich smart people"));
}

TEST(treeprinter, search_without_matches_still_prints_the_root) {
    auto tree = get_simple_tree();
    treelib::Search(tree).search("no such node");
    treelib::TreePrinter tree_printer(tree);

    auto lines = print_lines(tree_printer, tree.get_root(), 10, true);

    ASSERT_EQ(lines.size(), 1);
    ASSERT_TRUE(does_string_contain_substring(lines[0], "People"));
}

TEST(treeprinter, node_with_hidden_children_is_marked) {
    // A window of 3 lines has room for the root and its children, but not its grandchildren
    auto tree = get_simple_tree();
    treelib::TreePrinter tree_printer(tree);

    auto lines = print_lines(tree_printer, tree.get_root(), 3);

    ASSERT_TRUE(does_string_contain_substring(lines[1], "Dumb people/ (...)"));
    ASSERT_TRUE(does_string_contain_substring(lines[2], "Smart people/ (...)"));
}

TEST(treeprinter, siblings_that_dont_fit_are_counted) {
    auto tree = get_flat_tree(50);
    treelib::TreePrinter tree_printer(tree);

    auto lines = print_lines(tree_printer, tree.get_node("child000025"), 10);

    // The root, '... N more', some children, the selected one and '... M more'
    ASSERT_EQ(lines.size(), 10);
    int nr_hidden_at_beginning = 0, nr_hidden_at_end = 0;
    ASSERT_EQ(sscanf(lines[1].c_str(), "  \xe2\x94\x9c\xe2\x94\x80\xe2\x94\x80 ... %d more",
                     &nr_hidden_at_beginning), 1);
    ASSERT_EQ(sscanf(lines[9].c_str(), "  \xe2\x94\x94\xe2\x94\x80\xe2\x94\x80 ... %d more",
                     &nr_hidden_at_end), 1);
    const int nr_children_printed = lines.size() - 3;
    ASSERT_EQ(nr_hidden_at_beginning + nr_children_printed + nr_hidden_at_end, 50);
    ASSERT_TRUE(does_string_contain_substring(lines[8], "child000025"));
}

TEST(treeprinter, printed_lines_never_exceed_the_window) {
    for (int nr_children : {1, 2, 5, 9, 10, 11, 30}) {
        auto tree = get_flat_tree(nr_children);
        // From 4 lines: the root, and a selected child between two '... N more' rows
        for (int window_height = 4; window_height <= 12; ++window_height) {
            treelib::TreePrinter tree_printer(tree);
            for (auto& child : tree.get_root()->children) {
                SCOPED_TRACE("nr_children=" + std::to_string(nr_children) + " window_height=" +
                             std::to_string(window_height) + " selection=" + child->tag);
                auto lines = print_lines(tree_printer, child, window_height);
                ASSERT_LE(lines.size(), window_height);
                ASSERT_TRUE(tree_printer.was_selection_printed());
            }
        }
    }
}

TEST(treeprinter, selection_is_printed_in_tiny_windows) {
    auto tree = get_flat_tree(9);
    for (int window_height = 1; window_height <= 3; ++window_height) {
        treelib::TreePrinter tree_printer(tree);
        for (auto& child : tree.get_root()->children) {
            SCOPED_TRACE("window_height=" + std::to_string(window_height) + " selection=" +
                         child->tag);
            print_lines(tree_printer, child, window_height);
            ASSERT_TRUE(tree_printer.was_selection_printed());
        }
    }
}

TEST(treeprinter, print_deep_tree) {
    // Deeper than the 100 levels that get_node_depth() once asserted on
    auto tree = get_chain_tree(150);
    treelib::TreePrinter tree_printer(tree);

    auto lines = print_lines(tree_printer, tree.get_node("d150"), 10);

    ASSERT_LE(lines.size(), 10);
    ASSERT_TRUE(tree_printer.was_selection_printed());
    ASSERT_TRUE(does_string_contain_substring(lines.back(), "d150"));
}

TEST(treeprinter, breadcrumbs_that_fit) {
    auto tree = get_chain_tree(5);
    treelib::TreePrinter tree_printer(tree);
    std::ostringstream out;

    tree_printer.print(out, false, tree.get_node("d5"), 3, 80);

    auto lines = split_string_by_newline(strip_escape_codes(out.str()));
    ASSERT_EQ(lines[0], " d0 > d1 > d2 > d3 > d4 > ");
}

TEST(treeprinter, breadcrumbs_too_wide_drop_the_dirs_closest_to_the_root) {
    auto tree = get_chain_tree(50);
    treelib::TreePrinter tree_printer(tree);
    std::ostringstream out;

    tree_printer.print(out, false, tree.get_node("d50"), 3, 30);

    auto lines = split_string_by_newline(strip_escape_codes(out.str()));
    // The selection marker column, then the breadcrumbs
    ASSERT_LE(lines[0].size(), 30);
    ASSERT_EQ(lines[0].substr(0, 7), " ... > ");
    ASSERT_EQ(lines[0].substr(lines[0].size() - 6), "d49 > ");
}

TEST(treeprinter, breadcrumbs_without_width_limit) {
    auto tree = get_chain_tree(50);
    treelib::TreePrinter tree_printer(tree);
    std::ostringstream out;

    tree_printer.print(out, false, tree.get_node("d50"), 3);

    auto lines = split_string_by_newline(strip_escape_codes(out.str()));
    ASSERT_EQ(lines[0].substr(0, 8), " d0 > d1");
}

TEST(treeprinter, picked_nodes_are_marked) {
    auto tree = get_simple_tree();
    tree.get_node("dumb")->is_picked = true;
    tree.get_node("poor-dumb")->is_picked = true;
    treelib::TreePrinter tree_printer(tree);

    auto lines = print_lines(tree_printer, tree.get_root(), 10);

    // The selection marker column, then the picked marker column
    ASSERT_EQ(lines[1].substr(0, 2), " X");
    ASSERT_EQ(lines[2].substr(0, 2), " X");
    ASSERT_EQ(lines[3].substr(0, 2), "  ");
    ASSERT_EQ(lines[4].substr(0, 2), "  ");
}

TEST(treeprinter, search_matches_are_highlighted) {
    auto tree = get_simple_tree();
    treelib::Search(tree).search("Rich", false);
    treelib::TreePrinter tree_printer(tree);
    std::ostringstream out;
    tree_printer.print(out, false, tree.get_root(), 10);

    // Yellow, before the matching names only
    const std::string yellow = "\x1B[33m";
    auto lines = split_string_by_newline(out.str());
    ASSERT_EQ(lines.size(), 7);
    for (auto& line : lines) {
        const bool is_match = does_string_contain_substring(line, "Rich");
        ASSERT_EQ(does_string_contain_substring(line, yellow + (is_match ? "Rich" : "")), is_match)
            << line;
    }
}

TEST(treeprinter, partly_picked_nodes_are_marked) {
    auto tree = get_simple_tree();
    tree.get_node("dumb")->is_partly_picked = true;
    tree.get_node("poor-dumb")->is_picked = true;
    treelib::TreePrinter tree_printer(tree);
    std::ostringstream out;
    tree_printer.print(out, false, tree.get_root(), 10);

    auto lines = split_string_by_newline(strip_escape_codes(out.str()));
    ASSERT_EQ(lines[1].substr(0, 2), " -");
    ASSERT_EQ(lines[2].substr(0, 2), " X");
    // In magenta
    ASSERT_TRUE(does_string_contain_substring(split_string_by_newline(out.str())[1],
                                              "\x1B[35m\x1B[1mDumb people"));
}

TEST(treeprinter, a_picked_roots_color_ends_with_its_line) {
    auto tree = get_simple_tree();
    tree.get_root()->is_partly_picked = true;
    treelib::TreePrinter tree_printer(tree);
    std::ostringstream out;
    tree_printer.print(out, false, tree.get_node("dumb"), 10);

    auto lines = split_string_by_newline(out.str());
    ASSERT_TRUE(does_string_contain_substring(lines[0], "\x1B[35m"));
    // Back to the default color before anything of the next line
    ASSERT_EQ(lines[1].substr(0, 8), "\x1B[39;49m");
}

TEST(treeprinter, an_empty_dir_looks_like_a_dir) {
    treelib::Tree tree;
    tree.create_node("root", "root", "");
    tree.create_node("empty", "empty", "root")->kind = 'd';
    tree.create_node("file", "file", "root")->kind = 'f';
    treelib::TreePrinter tree_printer(tree);
    std::ostringstream out;
    tree_printer.print(out, false, tree.get_root(), 10);

    auto lines = split_string_by_newline(out.str());
    // Dirs are bold
    ASSERT_TRUE(does_string_contain_substring(lines[1], "\x1B[1mempty"));
    ASSERT_FALSE(does_string_contain_substring(lines[2], "\x1B[1m"));
}

TEST(treeprinter, dirs_end_with_a_slash) {
    // Like ls -F: dirs, known as dirs or by their children, end with '/'; files don't
    treelib::Tree tree;
    tree.create_node("root", "root", "");
    tree.create_node("empty", "empty", "root")->kind = 'd';
    tree.create_node("full", "full", "root");
    tree.create_node("file", "file", "full")->kind = 'f';
    tree.create_node("untyped", "untyped", "root");
    treelib::TreePrinter tree_printer(tree);

    auto lines = print_lines(tree_printer, tree.get_root(), 10);

    ASSERT_TRUE(does_string_contain_substring(lines[1], " empty/"));
    ASSERT_TRUE(does_string_contain_substring(lines[2], " full/"));
    ASSERT_TRUE(does_string_contain_substring(lines[3], " file"));
    ASSERT_FALSE(does_string_contain_substring(lines[3], "/"));
    // Neither known as a dir nor with children: a file
    ASSERT_FALSE(does_string_contain_substring(lines[4], "/"));
}
