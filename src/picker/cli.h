#ifndef PICKER_CLI_H__
#define PICKER_CLI_H__

#include <istream>
#include <string>
#include <vector>

#include "picker/picker_options.h"
#include "tree/tree.h"

namespace picker {

// The command line options that every picker program has, so they all work the same way: the
// picker's own, which are as they are given (-m is is_multi, --header TEXT header, --label TEXT with
// an optional --label-color COLOR labels, --key KEY TEXT key_hints, --help-line TEXT help_lines,
// --updates FD updates_fd, --expect KEYS expect_keys (see CLI_OPTIONS_USAGE), --preview FD preview_fd,
// --selection FD selection_fd, --notify KEYS and --notify-fd FD notify_keys and notify_fd, ...), and
// those that the program uses to make its tree and options
struct CliOptions : PickerOptions {
    // -0: paths (on stdin, and the picked ones printed) end with '\0' rather than a newline
    bool is_null_separated = false;
    // --root NAME: the name shown for the tree's root (e.g. a stdin tree's, which is ".")
    std::string root_name;
    // --list: the input is a flat list of options (one per line), rather than paths
    bool is_list = false;
    // --typed: each path on stdin starts with its type and a space, like find -printf '%y %p'
    bool is_typed = false;
    // --picked FILE: with -m, start with the paths in FILE (one per line, or with -0 ending with
    // '\0') picked, like what treepicker printed
    std::string picked_file;
    // --max-depth N: when scanning a dir, go at most N levels below it (-1: no limit)
    int max_depth = -1;
    // --dirs-only: a dir's scan (or a --typed tree) has only its dirs, not its files
    bool is_dirs_only = false;
    // --lazy: scan a dir a dir at a time, only what's shown, rather than all of it at the start
    bool is_lazy = false;
    // --exclude PATH, each: a dir's scan leaves it out, with all in it (see PathTree::excluded_paths)
    std::vector<std::string> excluded_paths;
    // --select PATH: the node to start with selected ("." for the root)
    std::string selected_path;
    // --print-selection: print the selected node's path before the picked ones, so the caller
    // can show the tree again where it was
    bool is_selection_printed = false;
    // --side FILE: the side tree's paths (as stdin's, --typed), shown above the preview, in the column;
    // --side-root NAME, its root's name; --side-select FD, where to read the paths to select in
    // it, side_select_fd; --side-key KEY, printed when Enter is pressed in it, side_key (see
    // PickerOptions)
    std::string side_file;
    std::string side_root_name;
    // --states FILE: the nodes' states (see PickerOptions::node_states), read into node_states
    // once the tree is there
    std::string states_file;

    bool is_help_requested = false;
    // Set if the command line isn't valid
    std::string error;
    // What's left: the program's own arguments
    std::vector<std::string> args;
};

// The options' part of a program's usage text
extern const std::string CLI_OPTIONS_USAGE;

CliOptions parse_cli(int argc, char* argv[]);

// Let the user pick from tree, and print the ids of what was picked (the root's as "."), one per
// line, in the tree's order. Returns the program's exit code: 0 if something was picked, 1 if the user quit.
int pick_and_print(treelib::Tree& tree, const CliOptions& options);

// A warning that scanning dir couldn't read everything, listing the first few of its errors,
// or "" if there are none
std::string scan_errors_warning(const std::string& dir, const std::vector<std::string>& errors);

// The index of the group named name in options.groups, added if it isn't there
int group_index(PickerOptions& options, const std::string& name);

// Read --states' file: each line (ending with delimiter) the node's allowed states (their names,
// separated by commas; empty: all), a tab, the state it starts in (empty: the first), a tab, its
// group (empty: none), a tab and its path (as printed). Returns an error, or "".
std::string read_node_states(CliOptions& options, std::istream& in, char delimiter);

// Add a child to the root for each line in `in` (ending with delimiter), its id and tag the line
// itself. Empty lines and repeated ones are skipped.
void add_list_from_stream(treelib::Tree& tree, std::istream& in, char delimiter);

} // namespace picker

#endif
