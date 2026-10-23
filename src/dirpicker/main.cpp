#include <locale.h>
#include <cstring>
#include <iostream>
#include <string>
#include "picker/picker.h"
#include "dirtree/dirtree.h"

static const char USAGE[] =
    "Usage: dirpicker [-0] [DIR | -]\n"
    "Pick a node of a tree of paths.\n"
    "\n"
    "  DIR  the dir to pick from (default: the current dir)\n"
    "  -    read the tree from stdin: one path per line, relative to the tree's root, with '/'\n"
    "       between dirs. Dirs that aren't listed (e.g. \"a\" for \"a/b\") are added.\n"
    "  -0   paths on stdin end with '\\0' rather than a newline\n";

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");

    std::string dir = ".";
    bool is_null_separated = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" or arg == "--help") {
            std::cout << USAGE;
            return 0;
        } else if (arg == "-0") {
            is_null_separated = true;
        } else if (arg.size() > 1 and arg[0] == '-') {
            std::cerr << "dirpicker: unknown option '" << arg << "'\n\n" << USAGE;
            return 2;
        } else {
            dir = arg;
        }
    }

    try {
        dirtree::DirTree tree(dir);
        if (dir == "-") {
            std::ios::sync_with_stdio(false);
            tree.update_from_stream(std::cin, is_null_separated ? '\0' : '\n');
        } else {
            tree.update_from_filesystem();
        }
        picker::Picker picker(tree);
        picker.run();
    } catch (const std::exception& e) {
        std::cerr << "dirpicker: " << e.what() << "\n";
        return 2;
    }

    return 0;
}
