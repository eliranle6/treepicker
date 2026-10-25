#ifndef PICKER_CLI_H__
#define PICKER_CLI_H__

#include <istream>
#include <string>
#include <vector>

#include "tree/tree.h"

namespace picker {

// The command line options that every picker program has, so they all work the same way
struct CliOptions {
    // -0: paths (on stdin, and the picked ones printed) end with '\0' rather than a newline
    bool is_null_separated = false;
    // -m, --multi: pick several nodes
    bool is_multi = false;
    // --header TEXT: lines shown above the tree
    std::string header;
    // --root NAME: the name shown for the tree's root (e.g. a stdin tree's, which is ".")
    std::string root_name;
    // --label TEXT, --label-color COLOR: a label at the start of the stats line
    std::string label;
    std::string label_color = "blue";
    // --list: the input is a flat list of options (one per line), rather than paths
    bool is_list = false;
    // --typed: each path on stdin starts with its type and a space, like find -printf '%y %p'
    bool is_typed = false;
    // --picked FILE: with -m, start with the paths in FILE (one per line, or with -0 ending with
    // '\0') picked, like what treepick printed
    std::string picked_file;
    // --max-depth N: when scanning a dir, go at most N levels below it (-1: no limit)
    int max_depth = -1;
    // --updates FD: read changes to the nodes' shown names from FD while picking (-1: none)
    int updates_fd = -1;

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

// A warning that scanning dir couldn't read everything, listing the first few of find's errors,
// or "" if there are none
std::string scan_errors_warning(const std::string& dir, const std::vector<std::string>& errors);

// Add a child to the root for each line in `in` (ending with delimiter), its id and tag the line
// itself. Empty lines and repeated ones are skipped.
void add_list_from_stream(treelib::Tree& tree, std::istream& in, char delimiter);

} // namespace picker

#endif
