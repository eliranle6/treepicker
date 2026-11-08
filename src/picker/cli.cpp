#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "picker/cli.h"
#include "pathtree/pathtree.h"
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
    "  --footer TEXT  show TEXT (one or more lines, plain or in styled parts as --preview's)\n"
    "                 below the tree, above the bottom lines, e.g. a warning\n"
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
    "                 a line for ?'s help, after the keys (e.g. what the caller's tags mean;\n"
    "                 one not indented that ends with ':' is a heading)\n"
    "  --picked FILE  with -m, start with the nodes whose paths are in FILE (as printed) picked\n"
    "  --updates FD   while picking, read changes to what nodes are shown as from file\n"
    "                 descriptor FD: each is the node's path (or option), a tab, and what\n"
    "                 to show, ending like the paths do (see -0). It's still printed by its\n"
    "                 path when picked. With a tab and 'disabled' after what to show, the node\n"
    "                 is shown in grey and can't be picked, until an update without them (with\n"
    "                 'skipped' rather than 'disabled', moving skips it too, e.g. a heading); with\n"
    "                 a tab and color=NAME before them (red, green, yellow, blue, magenta, cyan),\n"
    "                 its text is in that color, unless it's selected or picked. What to show\n"
    "                 may be in styled parts: after \\x02, a style and \\x02, in that style (its\n"
    "                 words: one of those colors, grey or orange, and bold; empty: plain again),\n"
    "                 unless it's selected, picked or matching a search. With\n"
    "                 an empty path, what to show is the caller's status: a yellow label on the\n"
    "                 bottom line (empty: none); or labels of other colors, each part starting\n"
    "                 with \\x01, a --label-color and \\x01 a label of that color.\n"
    "  --expect KEYS  keys (letters, digits, punctuation or control keys, e.g. Ctrl-R) that\n"
    "                 finish picking too, so the\n"
    "                 caller can act on the selected node: then the first line printed is\n"
    "                 the key pressed (or an empty line for Enter), the selected node's path\n"
    "                 after it, then the picked ones (with -m). Not keys treepicker uses.\n"
    "  --expect-twice KEYS\n"
    "                 keys that finish picking as --expect's do, when pressed twice in a row (as\n"
    "                 vim's cc); the key is printed once\n"
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
    "                 starting with \\x01, a --label-color and \\x01 is shown bold on that color;\n"
    "                 another may be in styled parts, as --updates' are (\\x02).\n"
    "                 One starting with \\x03 is an image, shown below the node's text in the\n"
    "                 rows it leaves, or if it's portrait, beside it on the column's right half\n"
    "                 (with 256 colors, two pixels a character): \\x03, the\n"
    "                 node's path, a tab, its width, a space, its height, a space and its\n"
    "                 pixels, row by row, each RRGGBB in hex (scaled down to fit); with nothing\n"
    "                 after the tab, the node's image is gone.\n"
    "  --preview-below\n"
    "                 show --preview's text below the tree, above the bottom lines, rather than\n"
    "                 in a column (at most 4 lines), each line in parts: plain, then after \\x01,\n"
    "                 a --label-color and \\x01, on that color (an empty color: plain again)\n"
    "  --side FILE    another tree to look through beside it, in the upper part of the right\n"
    "                 column (below --preview's text): its paths, as -'s with --typed, ending like\n"
    "                 the paths do (see -0). Ctrl-W l moves in it, Ctrl-W h (or Esc) back.\n"
    "  --side-root NAME\n"
    "                 the name shown for the side tree's root\n"
    "  --side-select FD\n"
    "                 read paths of the side tree's nodes to select from file descriptor FD while\n"
    "                 picking (e.g. where the selected node would go), ending like the paths do,\n"
    "                 unless the user is moving in the side tree\n"
    "  --side-start PATH\n"
    "                 start moving in the side tree, on the node at PATH (as printed), or its\n"
    "                 closest ancestor there\n"
    "  --side-key KEY Enter on a side tree's node finishes picking, printing KEY (as an --expect\n"
    "                 key), the selected node's path, the side tree's selected node's, then the\n"
    "                 picked ones\n"
    "  --side-expect KEYS\n"
    "                 keys that do as Enter does there (--side-key is printed), printing after the\n"
    "                 side tree's node the key pressed (nothing for Enter)\n"
    "  --side-hint KEY TEXT\n"
    "                 the keys row says it, while moving in the side tree\n"
    "  --key-if-none-picked KEY TEXT\n"
    "                 the keys row says it of a key it lists, while nothing's picked\n"
    "  --state NAME MARK COLOR KEY\n"
    "                 with -m, nodes are in states rather than picked or not: given once for\n"
    "                 each, in order, the first (e.g. skip) being unpicked. MARK (one column) is\n"
    "                 shown before a node in that state, in COLOR (a --label-color, grey or\n"
    "                 orange), and KEY (or \"\" for none) sets the selection's (a dir's: all under\n"
    "                 it that allow it). Space sets the next one it allows. Printed: each node\n"
    "                 without children not in the first state, as its state's NAME, a tab and\n"
    "                 its path.\n"
    "  --state-mark NAME GROUP MARK COLOR\n"
    "                 show state NAME by MARK in COLOR for GROUP's nodes (see --states)\n"
    "  --states FILE  the nodes' states: each line (ending like the paths do) the states the\n"
    "                 node allows (names separated by commas; empty: all), a tab, the one it\n"
    "                 starts in (empty: the first), a tab, its group (a name; empty: none), a tab\n"
    "                 and its path (as printed)\n"
    "  --tab TITLE GROUPS\n"
    "                 a tab, in a row above the tree, that shows only the nodes of GROUPS (their\n"
    "                 names separated by commas; empty: all), with their count. Given again, for\n"
    "                 more tabs; Tab and Shift-Tab switch.\n"
    "  -0             paths end with '\\0' rather than a newline (read ones and printed ones)\n"
    "  -h, --help     show this help\n"
    "\n"
    "Keys:\n") + keys_help() +
    "\n"
    "Prints the picked nodes' paths, in the tree's order. Exits with 0 if a node was picked (or an\n"
    "--expect key was pressed), 1 if not.\n";

// Whether treepicker uses the key (a byte) itself, when moving around
static bool is_treepickers_key(unsigned char byte) {
    for (auto& command : TreeKeyboardSelector::get_key_commands()) {
        for (auto& binding : command.bindings) {
            if (binding.first == byte) {
                return true;
            }
        }
    }
    return false;
}

// Why key can't be an --expect key, or "" if it can: it must be printable, or a control key (e.g.
// Ctrl-R), and not one treepicker uses
static std::string expect_key_error(char key) {
    const auto byte = static_cast<unsigned char>(key);
    if (byte == 0 or byte == ' ' or byte > '~') {
        return "--expect keys must be letters, digits, punctuation or control keys";
    }
    if (is_treepickers_key(byte)) {
        return byte < ' ' ? "--expect key " + std::to_string(byte) + " is one of treepicker's keys" :
            std::string("--expect key '") + key + "' is one of treepicker's keys";
    }
    return "";
}

// Why key can't be a --notify key, or "" if it can: any byte (a control key too, e.g. Tab, which is
// Ctrl-I) but treepicker's own keys
static std::string notify_key_error(char key) {
    const auto byte = static_cast<unsigned char>(key);
    return is_treepickers_key(byte) ? "--notify key " + std::to_string(byte) + " is one of treepicker's keys" : "";
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
        } else if (arg == "--side-hint" or arg == "--key-if-none-picked") {
            if (i + 2 >= argc) {
                options.error = arg + " needs a key and what it does";
            } else {
                (arg == "--side-hint" ? options.side_key_hints : options.none_picked_key_hints).push_back(
                    {argv[i + 1], argv[i + 2]});
                i += 2;
            }
        } else if (arg == "--side-expect") {
            if (i + 1 == argc) {
                options.error = "--side-expect needs a value";
            } else {
                options.side_expect_keys = argv[++i];
                for (char key : options.side_expect_keys) {
                    if (not expect_key_error(key).empty()) {
                        options.error = expect_key_error(key);
                    }
                }
            }
        } else if (arg == "--help-line") {
            if (i + 1 == argc) {
                options.error = "--help-line needs a value";
            } else {
                options.help_lines.push_back(argv[++i]);
            }
        } else if (arg == "--header" or arg == "--footer" or arg == "--exclude" or arg == "--root" or arg == "--picked" or arg == "--label" or
                   arg == "--label-color" or arg == "--expect" or arg == "--select" or arg == "--notify" or
                   arg == "--expect-twice" or arg == "--side-start") {
            if (i + 1 == argc) {
                options.error = arg + " needs a value";
            } else if (arg == "--expect" or arg == "--expect-twice") {
                (arg == "--expect" ? options.expect_keys : options.expect_twice_keys) += argv[++i];
                for (char key : std::string(argv[i])) {
                    if (options.error.empty()) {
                        options.error = expect_key_error(key);
                    }
                }
            } else if (arg == "--select") {
                options.selected_path = argv[++i];
            } else if (arg == "--side-start") {
                options.side_start_path = argv[++i];
            } else if (arg == "--exclude") {
                options.excluded_paths.push_back(argv[++i]);
            } else if (arg == "--notify") {
                options.notify_keys += argv[++i];
                for (char key : std::string(argv[i])) {
                    if (options.error.empty()) {
                        options.error = notify_key_error(key);
                    }
                }
            } else if (arg == "--header") {
                options.header = argv[++i];
            } else if (arg == "--footer") {
                options.footer = argv[++i];
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
        } else if (arg == "--state" or arg == "--state-mark") {
            if (i + 4 >= argc) {
                options.error = arg + " needs four values";
            } else if (arg == "--state") {
                StateOption state{argv[i + 1], argv[i + 2], argv[i + 3]};
                const std::string key = argv[i + 4];
                if (key.size() > 1) {
                    options.error = "--state needs one key, or \"\"";
                } else if (key.size() == 1) {
                    state.key = static_cast<unsigned char>(key[0]);
                    const std::string error = expect_key_error(key[0]);
                    if (not error.empty()) {
                        options.error = "--state key '" + key + "' can't be used: " + error;
                    }
                }
                if (not guishell::is_label_color(state.color) and state.color != "grey" and state.color != "orange") {
                    options.error = "unknown --state color '" + state.color + "'";
                }
                options.states.push_back(state);
                i += 4;
            } else {
                const std::string name = argv[i + 1];
                auto state = std::find_if(options.states.begin(), options.states.end(),
                                          [&](const StateOption& s) { return s.name == name; });
                if (state == options.states.end()) {
                    options.error = "--state-mark of an unknown state '" + name + "' (give its --state first)";
                } else {
                    state->group_marks.push_back({group_index(options, argv[i + 2]), argv[i + 3], argv[i + 4]});
                }
                i += 4;
            }
        } else if (arg == "--states") {
            if (i + 1 == argc) {
                options.error = arg + " needs a value";
            } else {
                options.states_file = argv[++i];
            }
        } else if (arg == "--tab") {
            if (i + 2 >= argc) {
                options.error = "--tab needs a title and groups";
            } else {
                TabOption tab{argv[i + 1], {}};
                std::istringstream groups(argv[i + 2]);
                for (std::string group; std::getline(groups, group, ',');) {
                    if (not group.empty()) {
                        tab.groups.push_back(group_index(options, group));
                    }
                }
                options.tabs.push_back(tab);
                i += 2;
            }
        } else if (arg == "--side" or arg == "--side-root" or arg == "--side-key") {
            if (i + 1 == argc) {
                options.error = arg + " needs a value";
            } else if (arg == "--side") {
                options.side_file = argv[++i];
            } else if (arg == "--side-root") {
                options.side_root_name = argv[++i];
            } else {
                const std::string key = argv[++i];
                options.side_key = key.size() == 1 ? static_cast<unsigned char>(key[0]) : 0;
                if (options.side_key == 0) {
                    options.error = "--side-key needs one key";
                }
            }
        } else if (arg == "--max-depth" or arg == "--updates" or arg == "--preview" or
                   arg == "--selection" or arg == "--notify-fd" or arg == "--side-select") {
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
            } else if (arg == "--side-select") {
                options.side_select_fd = value;
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
        } else if (arg == "--preview-below") {
            options.is_preview_below = true;
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
    if (options.error.empty() and not options.states.empty() and not options.is_multi) {
        options.error = "--state needs -m";
    }
    if (options.error.empty() and options.states.empty() and not options.states_file.empty()) {
        options.error = "--states needs --state";
    }
    return options;
}

int group_index(PickerOptions& options, const std::string& name) {
    auto group = std::find(options.groups.begin(), options.groups.end(), name);
    if (group != options.groups.end()) {
        return static_cast<int>(group - options.groups.begin());
    }
    options.groups.push_back(name);
    return static_cast<int>(options.groups.size()) - 1;
}

// The index of the state named name, or -1
static int state_index(const PickerOptions& options, const std::string& name) {
    for (std::size_t i = 0; i < options.states.size(); ++i) {
        if (options.states[i].name == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

std::string read_node_states(CliOptions& options, std::istream& in, char delimiter) {
    for (std::string line; std::getline(in, line, delimiter);) {
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> fields;
        std::size_t start = 0;
        // Three tabs: the path after the last may have tabs of its own
        for (int i = 0; i < 3; ++i) {
            const std::size_t tab = line.find('\t', start);
            if (tab == std::string::npos) {
                return "--states: a line without its four fields: " + line;
            }
            fields.push_back(line.substr(start, tab - start));
            start = tab + 1;
        }
        NodeStates node;
        node.id = line.substr(start);
        if (not fields[0].empty()) {
            node.allowed = 0;
            std::istringstream names(fields[0]);
            for (std::string name; std::getline(names, name, ',');) {
                const int state = state_index(options, name);
                if (state < 0) {
                    return "--states: an unknown state '" + name + "'";
                }
                node.allowed |= 1u << state;
            }
        }
        node.initial = fields[1].empty() ? 0 : state_index(options, fields[1]);
        if (node.initial < 0) {
            return "--states: an unknown state '" + fields[1] + "'";
        }
        node.group = fields[2].empty() ? -1 : group_index(options, fields[2]);
        options.node_states.push_back(node);
    }
    return "";
}

int pick_and_print(treelib::Tree& tree, const CliOptions& options) {
    const char delimiter = options.is_null_separated ? '\0' : '\n';
    if (not options.root_name.empty() and tree.get_root() != nullptr) {
        tree.get_root()->tag = options.root_name;
    }
    // What's given as it is; what isn't, worked out here
    PickerOptions picker_options = options;
    // A list's root is only its title
    picker_options.is_first_child_selected_first = options.is_list;
    picker_options.is_root_pickable = not options.is_list;
    picker_options.updates_delimiter = delimiter;
    if (not options.selected_path.empty() and tree.get_root() != nullptr) {
        picker_options.selected_id = options.selected_path == "." ? tree.get_root()->identifier :
            options.selected_path;
    }
    if (not options.states_file.empty()) {
        std::ifstream states_file(options.states_file);
        if (not states_file) {
            throw std::runtime_error("Cannot read " + options.states_file);
        }
        // Into a copy: the groups' names are added to as they're read
        CliOptions with_states = options;
        const std::string error = read_node_states(with_states, states_file, delimiter);
        if (not error.empty()) {
            throw std::runtime_error(error);
        }
        picker_options.node_states = with_states.node_states;
        picker_options.groups = with_states.groups;
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
    // The side tree, from its file (as stdin's tree is read, typed)
    pathtree::PathTree side_tree("-");
    if (not options.side_file.empty()) {
        std::ifstream side_file(options.side_file);
        if (not side_file) {
            throw std::runtime_error("Cannot read " + options.side_file);
        }
        side_tree.update_from_stream(side_file, delimiter, true);
        if (side_tree.get_root() != nullptr) {
            side_tree.get_root()->tag = options.side_root_name.empty() ? "." : options.side_root_name;
        }
        picker_options.side_tree = &side_tree;
    }
    Picker picker(tree, picker_options);
    const auto picked = picker.run();
    const auto path = [&](const std::shared_ptr<treelib::Node>& node) {
        return node == tree.get_root() ? std::string(".") : node->identifier;
    };
    // The screen is the terminal's, so stdout is only the picked nodes (and with --expect, the
    // key first)
    const int key = picker.get_expected_key();
    if (key != 0 or not picked.empty()) {
        if (not options.expect_keys.empty() or not options.expect_twice_keys.empty() or options.side_key != 0) {
            std::cout << (key != 0 ? std::string(1, static_cast<char>(key)) : "") << delimiter;
        }
        if (key != 0 or options.is_selection_printed) {
            std::cout << path(picker.get_selection()) << delimiter;
        }
        // Enter in the side tree: its selected node, after the tree's
        if (key != 0 and key == options.side_key and picker_options.side_tree != nullptr) {
            const auto side_selection = picker.get_side_selection();
            std::cout << (side_selection == side_tree.get_root() ? std::string(".") : side_selection->identifier)
                      << delimiter;
            if (not options.side_expect_keys.empty()) {
                const int pressed = picker.get_side_pressed_key();
                std::cout << (pressed != 0 ? std::string(1, static_cast<char>(pressed)) : "") << delimiter;
            }
        }
    }
    for (auto& node : picked) {
        // With states: the nodes without children, each with its state's name
        if (options.states.empty()) {
            std::cout << path(node) << delimiter;
        } else if (node->children.empty() and node->state > 0 and
                   static_cast<std::size_t>(node->state) < options.states.size()) {
            std::cout << options.states[node->state].name << '\t' << path(node) << delimiter;
        }
    }
    return picked.empty() and key == 0 ? 1 : 0;
}

std::string scan_errors_warning(const std::string& dir, const std::vector<std::string>& errors) {
    static const std::size_t MAX_ERRORS_SHOWN = 3;
    if (errors.empty()) {
        return "";
    }
    std::string warning = "Warning: couldn't read everything in " + dir + " (" +
        std::to_string(errors.size()) + (errors.size() == 1 ? " error" : " errors") +
        "), so the tree may lack files:";
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
