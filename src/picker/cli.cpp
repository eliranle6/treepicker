#include <iostream>

#include "picker/cli.h"
#include "picker/picker.h"

namespace picker {

const char CLI_OPTIONS_USAGE[] =
    "  -0          paths end with '\\0' rather than a newline (read ones and printed ones)\n"
    "  -h, --help  show this help\n"
    "\n"
    "Keys: h/j/k/l or arrows to move, / to search, Enter to pick, q to quit.\n"
    "Prints the picked node's path. Exits with 0 if a node was picked, 1 if not.\n";

CliOptions parse_cli(int argc, char* argv[]) {
    CliOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" or arg == "--help") {
            options.is_help_requested = true;
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
    const auto picked = Picker(tree).run();
    // The screen is the terminal's, so stdout is only the picked nodes
    const char delimiter = options.is_null_separated ? '\0' : '\n';
    for (auto& node : picked) {
        std::cout << (node == tree.get_root() ? "." : node->identifier) << delimiter;
    }
    return picked.empty() ? 1 : 0;
}

} // namespace picker
