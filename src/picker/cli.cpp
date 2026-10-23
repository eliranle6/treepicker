#include <iostream>

#include "picker/cli.h"
#include "picker/picker.h"

namespace picker {

const char CLI_OPTIONS_USAGE[] =
    "  -m, --multi    pick several nodes: Space picks the selected node and everything under\n"
    "                 it (or unpicks them), and Enter finishes\n"
    "  --header TEXT  show TEXT (one or more lines) above the tree\n"
    "  --root NAME    show the tree's root as NAME\n"
    "  -0             paths end with '\\0' rather than a newline (read ones and printed ones)\n"
    "  -h, --help     show this help\n"
    "\n"
    "Keys: h/j/k/l or arrows to move, / to search (Enter keeps the filter, Esc drops it),\n"
    "Enter to pick, q to quit.\n"
    "Prints the picked nodes' paths, in the tree's order. Exits with 0 if a node was picked, 1 if\n"
    "not.\n";

CliOptions parse_cli(int argc, char* argv[]) {
    CliOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" or arg == "--help") {
            options.is_help_requested = true;
        } else if (arg == "-m" or arg == "--multi") {
            options.is_multi = true;
        } else if (arg == "--header" or arg == "--root") {
            if (i + 1 == argc) {
                options.error = arg + " needs a value";
            } else {
                (arg == "--header" ? options.header : options.root_name) = argv[++i];
            }
        } else if (arg == "-0") {
            options.is_null_separated = true;
        } else if (arg.size() > 1 and arg[0] == '-') {
            options.error = "unknown option '" + arg + "'";
        } else {
            options.args.push_back(arg);
        }
    }
    return options;
}

int pick_and_print(treelib::Tree& tree, const CliOptions& options) {
    if (not options.root_name.empty() and tree.get_root() != nullptr) {
        tree.get_root()->tag = options.root_name;
    }
    PickerOptions picker_options;
    picker_options.is_multi = options.is_multi;
    picker_options.header = options.header;
    const auto picked = Picker(tree, picker_options).run();
    // The screen is the terminal's, so stdout is only the picked nodes
    const char delimiter = options.is_null_separated ? '\0' : '\n';
    for (auto& node : picked) {
        std::cout << (node == tree.get_root() ? "." : node->identifier) << delimiter;
    }
    return picked.empty() ? 1 : 0;
}

} // namespace picker
