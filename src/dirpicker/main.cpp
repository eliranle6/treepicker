#include <locale.h>
#include <iostream>
#include <string>
#include "picker/cli.h"
#include "dirtree/dirtree.h"

static const char USAGE[] =
    "Usage: dirpicker [OPTIONS] [DIR | -]\n"
    "Pick a node of a tree of paths.\n"
    "\n"
    "  DIR         the dir to pick from (default: the current dir)\n"
    "  -           read the tree from stdin: one path per line, relative to the tree's root,\n"
    "              with '/' between dirs. Dirs that aren't listed (e.g. \"a\" for \"a/b\") are\n"
    "              added.\n";

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");

    const auto options = picker::parse_cli(argc, argv);
    if (options.is_help_requested) {
        std::cout << USAGE << picker::CLI_OPTIONS_USAGE;
        return 0;
    }
    if (not options.error.empty() or options.args.size() > 1) {
        std::cerr << "dirpicker: " << (options.error.empty() ? "too many arguments" : options.error)
                  << "\n\n" << USAGE << picker::CLI_OPTIONS_USAGE;
        return 2;
    }
    const std::string dir = options.args.empty() ? "." : options.args[0];

    try {
        dirtree::DirTree tree(dir);
        if (dir == "-") {
            std::ios::sync_with_stdio(false);
            tree.update_from_stream(std::cin, options.is_null_separated ? '\0' : '\n');
        } else {
            tree.update_from_filesystem();
        }
        return picker::pick_and_print(tree, options);
    } catch (const std::exception& e) {
        std::cerr << "dirpicker: " << e.what() << "\n";
        return 2;
    }
}
