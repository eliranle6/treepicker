#include <locale.h>
#include <iostream>
#include <string>
#include "picker/cli.h"
#include "dirtree/dirtree.h"

static const char USAGE[] =
    "Usage: dirpicker [OPTIONS] [DIR | -]\n"
    "       dirpicker [OPTIONS] --list\n"
    "Pick a node of a tree of paths.\n"
    "\n"
    "  DIR            the dir to pick from (default: the current dir)\n"
    "  -              read the tree from stdin: one path per line, relative to the tree's root,\n"
    "                 with '/' between dirs. Dirs that aren't listed (e.g. \"a\" for \"a/b\")\n"
    "                 are added.\n"
    "  --list         read a list of options from stdin, one per line, and pick one (or, with\n"
    "                 -m, several). Prints the picked ones.\n";

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
    const std::string dir = options.args.empty() ? (options.is_list ? "-" : ".") : options.args[0];
    if (options.is_list and dir != "-") {
        std::cerr << "dirpicker: --list reads from stdin, so it takes no DIR\n";
        return 2;
    }

    try {
        dirtree::DirTree tree(dir);
        const char delimiter = options.is_null_separated ? '\0' : '\n';
        if (options.is_list) {
            std::ios::sync_with_stdio(false);
            picker::add_list_from_stream(tree, std::cin, delimiter);
        } else if (dir == "-") {
            std::ios::sync_with_stdio(false);
            tree.update_from_stream(std::cin, delimiter);
        } else {
            tree.update_from_filesystem();
        }
        return picker::pick_and_print(tree, options);
    } catch (const std::exception& e) {
        std::cerr << "dirpicker: " << e.what() << "\n";
        return 2;
    }
}
