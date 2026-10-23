#include <locale.h>
#include <iostream>
#include "tree/tree.h"
#include "picker/cli.h"

static const char USAGE[] =
    "Usage: picker [OPTIONS]\n"
    "Pick a node of an example tree.\n"
    "\n";

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");

    const auto options = picker::parse_cli(argc, argv);
    if (options.is_help_requested) {
        std::cout << USAGE << picker::CLI_OPTIONS_USAGE;
        return 0;
    }
    if (not options.error.empty() or not options.args.empty() or options.is_list) {
        std::cerr << "picker: " << (options.error.empty() ? "too many arguments" : options.error)
                  << "\n\n" << USAGE << picker::CLI_OPTIONS_USAGE;
        return 2;
    }

    treelib::Tree tree;
    tree.create_node("People", "people", "", nullptr);
        tree.create_node("Dumb people", "dumb", "people", nullptr);
            tree.create_node("Rich dumb people", "rich-dumb", "dumb", nullptr);
            tree.create_node("Poor dumb people", "poor-dumb", "dumb", nullptr);
        tree.create_node("Smart people", "smart", "people", nullptr);
            tree.create_node("Rich smart people", "rich-smart", "smart", nullptr);
            tree.create_node("Poor smart people", "poor-smart", "smart", nullptr);

    return picker::pick_and_print(tree, options);
}
