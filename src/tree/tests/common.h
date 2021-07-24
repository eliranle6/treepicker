#ifndef __COMMON_H
#define __COMMON_H

#include "tree/tree.h"

treelib::Tree get_simple_tree(void) {
    treelib::Tree tree;
    tree.create_node("People", "people", "", nullptr);
        tree.create_node("Dumb people", "dumb", "people", nullptr);
            tree.create_node("Rich dumb people", "rich-dumb", "dumb", nullptr);
            tree.create_node("Poor dumb people", "poor-dumb", "dumb", nullptr);
        tree.create_node("Smart people", "smart", "people", nullptr);
            tree.create_node("Rich smart people", "rich-smart", "smart", nullptr);
            tree.create_node("Poor smart people", "poor-smart", "smart", nullptr);
    return tree;
}

treelib::Tree get_simple_tree_informative_nodes(void) {
    treelib::Tree tree;
    tree.create_node("root", "root", "", nullptr);
    tree.create_node("child1", "child1", "root", nullptr);
    tree.create_node("child1.1", "child1.1", "child1", nullptr);
    tree.create_node("child1.2", "child1.2", "child1", nullptr);
    tree.create_node("child2", "child2", "root", nullptr);
    tree.create_node("child2.1", "child2.1", "child2", nullptr);
    return tree;
}

treelib::Tree get_tree_with_countries(void) {
    treelib::Tree tree;
    tree.create_node("Countries", "countries", "", nullptr);
        tree.create_node("Jewish majority countries", "jewish_majority", "countries", nullptr);
            tree.create_node("Israel", "israel", "jewish_majority", nullptr);
        tree.create_node("Non Jewish majority countries", "nonjewish_majority", "countries", nullptr);
            tree.create_node("Bangladesh", "bangladesh", "nonjewish_majority", nullptr);
            tree.create_node("India", "india", "nonjewish_majority", nullptr);
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

#endif
