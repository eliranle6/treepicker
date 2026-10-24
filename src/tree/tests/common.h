#ifndef __COMMON_H
#define __COMMON_H

#include <cstdio>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include "tree/tree.h"

treelib::Tree get_simple_tree(void) {
    treelib::Tree tree;
    tree.create_node("People", "people", "");
        tree.create_node("Dumb people", "dumb", "people");
            tree.create_node("Poor dumb people", "poor-dumb", "dumb");
            tree.create_node("Rich dumb people", "rich-dumb", "dumb");
        tree.create_node("Smart people", "smart", "people");
            tree.create_node("Poor smart people", "poor-smart", "smart");
            tree.create_node("Rich smart people", "rich-smart", "smart");
    return tree;
}

treelib::Tree get_simple_tree_informative_nodes(void) {
    treelib::Tree tree;
    tree.create_node("root", "root", "");
    tree.create_node("child1", "child1", "root");
    tree.create_node("child1.1", "child1.1", "child1");
    tree.create_node("child1.2", "child1.2", "child1");
    tree.create_node("child2", "child2", "root");
    tree.create_node("child2.1", "child2.1", "child2");
    return tree;
}

treelib::Tree get_tree_with_countries(void) {
    treelib::Tree tree;
    tree.create_node("Countries", "countries", "");
        tree.create_node("Jewish majority countries", "jewish_majority", "countries");
            tree.create_node("Israel", "israel", "jewish_majority");
        tree.create_node("Non Jewish majority countries", "nonjewish_majority", "countries");
            tree.create_node("Bangladesh", "bangladesh", "nonjewish_majority");
            tree.create_node("India", "india", "nonjewish_majority");
    return tree;
}

std::vector<std::string> split_string_by_newline(const std::string& str)
{
    auto result = std::vector<std::string>{};
    auto ss = std::stringstream{str};

    for (std::string line; std::getline(ss, line, '\n');)
        result.push_back(line);

    return result;
}


bool does_string_contain_substring(const std::string& string, const std::string& substring) {
    auto pos = string.find(substring) != std::string::npos;
    return pos;
}

// A root ("root") with nr_children leaves: "child000000", "child000001", ... (ids are the same
// as tags)
treelib::Tree get_flat_tree(int nr_children) {
    treelib::Tree tree;
    tree.create_node("root", "root", "");
    for (int i = 0; i < nr_children; ++i) {
        char tag[16];
        snprintf(tag, sizeof(tag), "child%06d", i);
        tree.create_node(tag, tag, "root");
    }
    return tree;
}

// A chain of nr_levels nodes under the root: "d1" (child of "d0", the root), "d2", ...
treelib::Tree get_chain_tree(int nr_levels) {
    treelib::Tree tree;
    tree.create_node("d0", "d0", "");
    for (int i = 1; i <= nr_levels; ++i) {
        tree.create_node("d" + std::to_string(i), "d" + std::to_string(i),
                         "d" + std::to_string(i - 1));
    }
    return tree;
}

// Printed text without its color/bold escape codes
std::string strip_escape_codes(const std::string& str) {
    static const std::regex escape_code("\x1B\\[[0-9;]*m");
    return std::regex_replace(str, escape_code, "");
}

#endif
