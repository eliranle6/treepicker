#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "dirtree/dirtree.h"

static const char USAGE[] =
    "Usage: dirdiff [-0] [--max-depth N] A B\n"
    "Print the paths (relative to A) of A's files that B doesn't have, by path only, in A's\n"
    "order (sorted by name). Both dirs are scanned with find; what find couldn't read is on\n"
    "stderr.\n"
    "\n"
    "  -0             end the paths with '\\0' rather than a newline\n"
    "  --max-depth N  scan at most N levels below each dir\n"
    "  -h, --help     show this help\n";

int main(int argc, char* argv[]) {
    bool is_null_separated = false;
    int max_depth = -1;
    std::vector<std::string> dirs;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" or arg == "--help") {
            std::cout << USAGE;
            return 0;
        } else if (arg == "-0") {
            is_null_separated = true;
        } else if (arg == "--max-depth" and i + 1 < argc) {
            char* end = nullptr;
            max_depth = std::strtol(argv[++i], &end, 10);
            if (end == argv[i] or *end != '\0' or max_depth < 0) {
                std::cerr << "dirdiff: --max-depth needs a number (0 or more)\n";
                return 2;
            }
        } else if (arg.size() > 1 and arg[0] == '-') {
            std::cerr << "dirdiff: unknown option '" << arg << "'\n\n" << USAGE;
            return 2;
        } else {
            dirs.push_back(arg);
        }
    }
    if (dirs.size() != 2) {
        std::cerr << "dirdiff: needs two dirs\n\n" << USAGE;
        return 2;
    }

    try {
        std::ios::sync_with_stdio(false);
        dirtree::DirTree a(dirs[0]);
        a.update_from_filesystem(max_depth);
        dirtree::DirTree b(dirs[1]);
        b.update_from_filesystem(max_depth);
        for (auto& error : a.scan_errors) {
            std::cerr << error << "\n";
        }
        for (auto& error : b.scan_errors) {
            std::cerr << error << "\n";
        }
        const char delimiter = is_null_separated ? '\0' : '\n';
        for (auto& path : a.files_not_in(b)) {
            std::cout << path << delimiter;
        }
    } catch (const std::exception& e) {
        std::cerr << "dirdiff: " << e.what() << "\n";
        return 2;
    }
    return 0;
}
