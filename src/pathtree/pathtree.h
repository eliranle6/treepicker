#ifndef __PATHTREE_H
#define __PATHTREE_H

#include <istream>
#include <string>
#include <vector>
#include <filesystem>

#include "tree/tree.h"
#include "utils/subprocess.h"

namespace pathtree {

class PathTree : public treelib::Tree
{
public:
    std::filesystem::path path;
    PathTree(std::string _path);

    // Add the dir's contents (found with `find`), sorted by name, down to max_depth levels below
    // the dir (-1: all)
    void update_from_filesystem(int max_depth = -1);

    // find's error messages from the last update_from_filesystem(), one per path it couldn't
    // read (e.g. "find: '/a/b': Permission denied"), so the tree may lack what's under them
    std::vector<std::string> scan_errors;

    // Add the paths read from in, each ending with delimiter (or the end of input), in the order
    // they're read. Paths are relative to the tree's root, '/'-separated; see add_path().
    // With is_typed, each starts with its type and a space, like find's %y: 'f' a file, 'd' a dir
    // (others are skipped, like symlinks in a dir's scan).
    void update_from_stream(std::istream& in, char delimiter = '\n', bool is_typed = false);

    // Add the node at path, after creating any of its ancestors that the tree doesn't have
    // yet. Empty and "." components are skipped (so "./a//b/" is "a/b"), and so are paths
    // already in the tree. The node's id is the cleaned path.
    // kind (see Node::kind) is set on the node, unless 0; dirs added for it are 'd'.
    void add_path(const std::string& path, char kind = 0);

    // The ids (paths) of this tree's files (nodes of kind 'f') that other doesn't have, in the
    // tree's order: what other lacks of this tree's files
    std::vector<std::string> files_not_in(treelib::Tree& other);

private:
    // Create the root node, unless there is one
    void add_root(const std::string& tag);
};

} // namespace pathtree

#endif
