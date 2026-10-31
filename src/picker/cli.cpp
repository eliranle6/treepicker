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
    "  --enter-picks-selection\n"
    "                 with -m, Enter with nothing picked picks the selected node (and everything\n"
    "                 under it, as Space does) and finishes, rather than doing nothing\n"
    "  --header TEXT  show TEXT (one or more lines) above the tree\n"
    "  --root NAME    show the tree's root as NAME\n"
    "  --label TEXT   show TEXT in bold at the start of the bottom line, e.g. a mode. Given\n"
    "                 again, the labels are shown side by side.\n"
    "  --label-color COLOR\n"
    "                 the color of the --label before it: red, orange, yellow, green, blue\n"
    "                 (default), magenta, cyan\n"
    "  --key KEY TEXT name KEY in the keys row at the bottom (and in ?'s help) as doing TEXT: a\n"
    "                 key of the caller's (e.g. an --expect or --notify key), or one of\n"
    "                 treepicker's, to say what it does here (e.g. --key Enter \"choose what to\n"
    "                 do\"). Given again, for more keys.\n"
    "  --help-line TEXT\n"
    "                 a line for ?'s help, after the keys (e.g. what the caller's tags mean)\n"
    "  --picked FILE  with -m, start with the nodes whose paths are in FILE (as printed) picked\n"
    "  --updates FD   while picking, read changes to what nodes are shown as from file\n"
    "                 descriptor FD: each is the node's path (or option), a tab, and what\n"
    "                 to show, ending like the paths do (see -0). It's still printed by its\n"
    "                 path when picked. With a tab and 'disabled' after what to show, the node\n"
    "                 is shown in grey and can't be picked, until an update without them. With\n"
    "                 an empty path, what to show is the caller's status: a yellow label on the\n"
    "                 bottom line (empty: none); or labels of other colors, each part starting\n"
    "                 with \\x01, a --label-color and \\x01 a label of that color.\n"
    "  --expect KEYS  keys (letters, digits or punctuation) that finish picking too, so the\n"
    "                 caller can act on the selected node: then the first line printed is\n"
    "                 the key pressed (or an empty line for Enter), the selected node's path\n"
    "                 after it, then the picked ones (with -m). Not keys treepicker uses.\n"
    "  --select PATH  start with the node at PATH (as printed) selected, rather than the root\n"
    "  --selection FD write the selected node's path (as printed) to file descriptor FD at the\n"
    "                 start and whenever the selection moves, ending like the paths do (see -0),\n"
    "                 e.g. for the caller to work out its --preview\n"
    "  --notify KEYS  keys (any, Tab (Ctrl-I) too, but treepicker's own) that tell the caller of the\n"
    "                 selection and the picks, without finishing picking: each writes to --notify-fd's\n"
    "                 file descriptor the key, the selected node's path, the picked ones' paths,\n"
    "                 each ending like the paths do (see -0), and an empty one to end it\n"
    "  --notify-fd FD the file descriptor --notify's keys write to\n"
    "  --print-selection\n"
    "                 print the selected node's path first (after --expect's key), whatever\n"
    "                 finished picking, so the caller can --select it again\n"
    "  --preview FD   show text beside the tree for the selected node, in a column on the right\n"
    "                 (p hides or shows it), read from file descriptor FD while picking (a\n"
    "                 file's or a pipe's): each is the node's path (as printed), a tab and the\n"
    "                 text's lines, separated by tabs, ending like the paths do (see -0). A\n"
    "                 later one replaces the node's text. With an empty path, it's the text of\n"
    "                 the nodes that have none (e.g. that it's still being worked out). A line\n"
    "                 starting with \\x01, a --label-color and \\x01 is shown bold on that color.\n"
    "  -0             paths end with '\\0' rather than a newline (read ones and printed ones)\n"
    "  -h, --help     show this help\n"
    "\n"
    "Keys:\n") + keys_help() +
    "\n"
    "Prints the picked nodes' paths, in the tree's order. Exits with 0 if a node was picked (or an\n"
    "--expect key was pressed), 1 if not.\n";

// Why key can't be an --expect key, or "" if it can: it must be printable, and not one treepicker
// uses
static std::string expect_key_error(char key) {
    if (key <= ' ' or key > '~') {
        return "--expect keys must be letters, digits or punctuation";
    }
    for (auto& command : TreeKeyboardSelector::get_key_commands()) {
        for (auto& binding : command.bindings) {
            if (binding.first == key) {
                return std::string("--expect key '") + key + "' is one of treepicker's keys";
            }
        }
    }
    return "";
}

// Why key can't be a --notify key, or "" if it can: any byte (a control key too, e.g. Tab, which is
// Ctrl-I) but treepicker's own keys
static std::string notify_key_error(char key) {
    for (auto& command : TreeKeyboardSelector::get_key_commands()) {
        for (auto& binding : command.bindings) {
            if (binding.first == static_cast<unsigned char>(key)) {
                return "--notify key " + std::to_string(static_cast<unsigned char>(key)) +
                    " is one of treepicker's keys";
            }
        }
    }
    return "";
}

CliOptions parse_cli(int argc, char* argv[]) {
    CliOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" or arg == "--help") {
            options.is_help_requested = true;
        } else if (arg == "-m" or arg == "--multi") {
            options.is_multi = true;
        } else if (arg == "--key") {
            if (i + 2 >= argc) {
                options.error = "--key needs a key and what it does";
            } else {
                options.key_hints.push_back({argv[i + 1], argv[i + 2]});
                i += 2;
            }
        } else if (arg == "--help-line") {
            if (i + 1 == argc) {
                options.error = "--help-line needs a value";
            } else {
                options.help_lines.push_back(argv[++i]);
            }
        } else if (arg == "--header" or arg == "--root" or arg == "--picked" or arg == "--label" or
                   arg == "--label-color" or arg == "--expect" or arg == "--select" or arg == "--notify") {
            if (i + 1 == argc) {
                options.error = arg + " needs a value";
            } else if (arg == "--expect") {
                options.expect_keys += argv[++i];
                for (char key : std::string(argv[i])) {
                    if (options.error.empty()) {
                        options.error = expect_key_error(key);
                    }
                }
            } else if (arg == "--select") {
                options.selected_path = argv[++i];
            } else if (arg == "--notify") {
                options.notify_keys += argv[++i];
                for (char key : std::string(argv[i])) {
                    if (options.error.empty()) {
                        options.error = notify_key_error(key);
                    }
                }
            } else if (arg == "--header") {
                options.header = argv[++i];
            } else if (arg == "--root") {
                options.root_name = argv[++i];
            } else if (arg == "--label") {
                options.labels.push_back({argv[++i]});
            } else if (arg == "--label-color") {
                const std::string color = argv[++i];
                if (options.labels.empty()) {
                    options.error = "--label-color goes after the --label it's for";
                } else if (not guishell::is_label_color(color)) {
                    options.error = "unknown --label-color '" + color + "'";
                } else {
                    options.labels.back().color = color;
                }
            } else {
                options.picked_file = argv[++i];
            }
        } else if (arg == "--max-depth" or arg == "--updates" or arg == "--preview" or
                   arg == "--selection" or arg == "--notify-fd") {
            char* end = nullptr;
            const long value = i + 1 < argc ? std::strtol(argv[i + 1], &end, 10) : -1;
            if (i + 1 == argc or end == argv[i + 1] or *end != '\0' or value < 0) {
                options.error = arg + " needs a number (0 or more)";
            } else if (arg == "--max-depth") {
                options.max_depth = value;
            } else if (arg == "--preview") {
                options.preview_fd = value;
            } else if (arg == "--selection") {
                options.selection_fd = value;
            } else if (arg == "--notify-fd") {
                options.notify_fd = value;
            } else {
                options.updates_fd = value;
            }
            ++i;
        } else if (arg == "--typed") {
            options.is_typed = true;
        } else if (arg == "--dirs-only") {
            options.is_dirs_only = true;
        } else if (arg == "--enter-picks-selection") {
            options.is_selection_picked_if_none = true;
        } else if (arg == "--print-selection") {
            options.is_selection_printed = true;
        } else if (arg == "--lazy") {
            options.is_lazy = true;
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
    picker_options.is_selection_picked_if_none = options.is_selection_picked_if_none;
    picker_options.header = options.header;
    picker_options.labels = options.labels;
    picker_options.key_hints = options.key_hints;
    picker_options.help_lines = options.help_lines;
    // A list's root is only its title
    picker_options.is_first_child_selected_first = options.is_list;
    picker_options.is_root_pickable = not options.is_list;
    picker_options.updates_fd = options.updates_fd;
    picker_options.updates_delimiter = delimiter;
    picker_options.expect_keys = options.expect_keys;
    if (not options.selected_path.empty() and tree.get_root() != nullptr) {
        picker_options.selected_id = options.selected_path == "." ? tree.get_root()->identifier :
            options.selected_path;
    }
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
    picker_options.preview_fd = options.preview_fd;
    picker_options.selection_fd = options.selection_fd;
    picker_options.notify_keys = options.notify_keys;
    picker_options.notify_fd = options.notify_fd;
    Picker picker(tree, picker_options);
    const auto picked = picker.run();
    const auto path = [&](const std::shared_ptr<treelib::Node>& node) {
        return node == tree.get_root() ? std::string(".") : node->identifier;
    };
    // The screen is the terminal's, so stdout is only the picked nodes (and with --expect, the
    // key first)
    const int key = picker.get_expected_key();
    if (key != 0 or not picked.empty()) {
        if (not options.expect_keys.empty()) {
            std::cout << (key != 0 ? std::string(1, static_cast<char>(key)) : "") << delimiter;
        }
        if (key != 0 or options.is_selection_printed) {
            std::cout << path(picker.get_selection()) << delimiter;
        }
    }
    for (auto& node : picked) {
        std::cout << path(node) << delimiter;
    }
    return picked.empty() and key == 0 ? 1 : 0;
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
