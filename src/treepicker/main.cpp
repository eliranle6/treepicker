#include <locale.h>
#include <iostream>
#include <string>
#include "picker/cli.h"
#include "pathtree/pathtree.h"

static const char USAGE[] =
    "Usage: treepicker [OPTIONS] [DIR | -]\n"
    "       treepicker [OPTIONS] --list\n"
    "Pick a node of a tree of paths.\n"
    "\n"
    "  DIR            the dir to pick from (default: the current dir)\n"
    "  --max-depth N  scan DIR at most N levels deep\n"
    "  --dirs-only    only dirs: DIR's (or, with - and --typed, the tree's) files aren't shown\n"
    "  --lazy         read DIR a dir at a time, only what's shown, rather than all of it at the\n"
    "                 start: for a huge DIR (e.g. /). A search finds only what was read. Not with\n"
    "                 -m, whose picks would miss what wasn't read.\n"
    "  -              read the tree from stdin: one path per line, relative to the tree's root,\n"
    "                 with '/' between dirs. Dirs that aren't listed (e.g. \"a\" for \"a/b\")\n"
    "                 are added.\n"
    "  --typed        with -, each path starts with its type and a space, like find's %y: 'f'\n"
    "                 a file, 'd' a dir (others are skipped). Counts picked files, not nodes.\n"
    "  --list         read a list of options from stdin, one per line, and pick one (or, with\n"
    "                 -m, several). Prints the picked ones.\n";

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "");

    auto options = picker::parse_cli(argc, argv);
    if (options.is_help_requested) {
        std::cout << USAGE << picker::CLI_OPTIONS_USAGE;
        return 0;
    }
    if (not options.error.empty() or options.args.size() > 1) {
        std::cerr << "treepicker: " << (options.error.empty() ? "too many arguments" : options.error)
                  << "\n\n" << USAGE << picker::CLI_OPTIONS_USAGE;
        return 2;
    }
    const std::string dir = options.args.empty() ? (options.is_list ? "-" : ".") : options.args[0];
    if (options.is_list and dir != "-") {
        std::cerr << "treepicker: --list reads from stdin, so it takes no DIR\n";
        return 2;
    }
    if (options.is_lazy and (dir == "-" or options.is_multi)) {
        std::cerr << "treepicker: --lazy reads a DIR, and not with -m\n";
        return 2;
    }

    try {
        pathtree::PathTree tree(dir);
        const char delimiter = options.is_null_separated ? '\0' : '\n';
        if (options.is_list) {
            std::ios::sync_with_stdio(false);
            picker::add_list_from_stream(tree, std::cin, delimiter);
        } else if (dir == "-") {
            std::ios::sync_with_stdio(false);
            tree.update_from_stream(std::cin, delimiter, options.is_typed, options.is_dirs_only);
        } else if (options.is_lazy) {
            tree.update_lazily_from_filesystem(options.max_depth, options.is_dirs_only);
        } else {
            tree.update_from_filesystem(options.max_depth, options.is_dirs_only);
            // Above the caller's header
            const auto warning = picker::scan_errors_warning(dir, tree.scan_errors);
            if (not warning.empty()) {
                options.header = warning + (options.header.empty() ? "" : "\n" + options.header);
            }
        }
        return picker::pick_and_print(tree, options);
    } catch (const std::exception& e) {
        std::cerr << "treepicker: " << e.what() << "\n";
        return 2;
    }
}
