#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "picker/cli.h"
#include "picker/keys_help.h"
#include "utils/guishell.h"
#include "picker/picker.h"

namespace picker {

const std::string CLI_OPTIONS_USAGE = std::string(
    "  -m, --multi    pick several nodes: Space picks the selected node and everything under\n"
    "                 it (or unpicks them), and Enter finishes\n"
    "  --header TEXT  show TEXT (one or more lines) above the tree\n"
    "  --root NAME    show the tree's root as NAME\n"
    "  --label TEXT   show TEXT in bold at the start of the bottom line, e.g. a mode\n"
    "  --label-color COLOR\n"
    "                 the label's color: red, green, yellow, blue (default), magenta, cyan\n"
    "  --picked FILE  with -m, start with the nodes whose paths are in FILE (as printed) picked\n"
    "  -0             paths end with '\\0' rather than a newline (read ones and printed ones)\n"
    "  -h, --help     show this help\n"
    "\n"
    "Keys:\n") + keys_help() +
    "\n"
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
        } else if (arg == "--header" or arg == "--root" or arg == "--picked" or arg == "--label" or
                   arg == "--label-color") {
            if (i + 1 == argc) {
                options.error = arg + " needs a value";
            } else if (arg == "--header") {
                options.header = argv[++i];
            } else if (arg == "--root") {
                options.root_name = argv[++i];
            } else if (arg == "--label") {
                options.label = argv[++i];
            } else if (arg == "--label-color") {
                options.label_color = argv[++i];
                if (not guishell::is_label_color(options.label_color)) {
                    options.error = "unknown --label-color '" + options.label_color + "'";
                }
            } else {
                options.picked_file = argv[++i];
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
        } else if (arg == "--typed") {
            options.is_typed = true;
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
    const char delimiter = options.is_null_separated ? '\0' : '\n';
    if (not options.root_name.empty() and tree.get_root() != nullptr) {
        tree.get_root()->tag = options.root_name;
    }
    PickerOptions picker_options;
    picker_options.is_multi = options.is_multi;
    picker_options.header = options.header;
    picker_options.label = options.label;
    picker_options.label_color = options.label_color;
    // A list's root is only its title
    picker_options.is_first_child_selected_first = options.is_list;
    picker_options.is_root_pickable = not options.is_list;
    if (not options.picked_file.empty()) {
        std::ifstream picked_file(options.picked_file);
        if (not picked_file) {
            throw std::runtime_error("Cannot read " + options.picked_file);
        }
        // As printed: the root is "."
        for (std::string path; std::getline(picked_file, path, delimiter);) {
            picker_options.initially_picked_ids.push_back(
                path == "." ? tree.get_root()->identifier : path);
        }
    }
    const auto picked = Picker(tree, picker_options).run();
    // The screen is the terminal's, so stdout is only the picked nodes
    for (auto& node : picked) {
        std::cout << (node == tree.get_root() ? "." : node->identifier) << delimiter;
    }
    return picked.empty() ? 1 : 0;
}

std::string scan_errors_warning(const std::string& dir, const std::vector<std::string>& errors) {
    static const std::size_t MAX_ERRORS_SHOWN = 3;
    if (errors.empty()) {
        return "";
    }
    std::string warning = "Warning: find couldn't read everything in " + dir + " (" +
        std::to_string(errors.size()) + " errors), so the tree may lack files:";
    for (std::size_t i = 0; i < errors.size() and i < MAX_ERRORS_SHOWN; ++i) {
        warning += "\n  " + errors[i];
    }
    if (errors.size() > MAX_ERRORS_SHOWN) {
        warning += "\n  (and " + std::to_string(errors.size() - MAX_ERRORS_SHOWN) + " more)";
    }
    return warning;
}

void add_list_from_stream(treelib::Tree& tree, std::istream& in, char delimiter) {
    if (tree.get_root() == nullptr) {
        tree.create_node(".", "", "");
    }
    const std::string root_id = tree.get_root()->identifier;
    std::string line;
    while (std::getline(in, line, delimiter)) {
        if (not line.empty()) {
            tree.get_or_create_node(line, line, root_id);
        }
    }
}

} // namespace picker
