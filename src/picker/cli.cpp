#include <cstdlib>
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
        } else if (arg == "--max-depth") {
            char* end = nullptr;
            const long value = i + 1 < argc ? std::strtol(argv[i + 1], &end, 10) : -1;
            if (i + 1 == argc or end == argv[i + 1] or *end != '\0' or value < 0) {
                options.error = "--max-depth needs a number (0 or more)";
            } else {
                options.max_depth = value;
            }
            ++i;
        } else if (arg == "--list") {
            options.is_list = true;
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
    // A list's root is only its title
    picker_options.is_first_child_selected_first = options.is_list;
    picker_options.is_root_pickable = not options.is_list;
    const auto picked = Picker(tree, picker_options).run();
    // The screen is the terminal's, so stdout is only the picked nodes
    const char delimiter = options.is_null_separated ? '\0' : '\n';
    for (auto& node : picked) {
        std::cout << (node == tree.get_root() ? "." : node->identifier) << delimiter;
    }
    return picked.empty() ? 1 : 0;
}

void add_list_from_stream(treelib::Tree& tree, std::istream& in, char delimiter) {
    if (tree.get_root() == nullptr) {
        tree.create_node(".", "", "", nullptr);
    }
    const std::string root_id = tree.get_root()->identifier;
    std::string line;
    while (std::getline(in, line, delimiter)) {
        if (not line.empty()) {
            tree.get_or_create_node(line, line, root_id, nullptr);
        }
    }
}

} // namespace picker
