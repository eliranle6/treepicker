#ifndef __DIRTREE_H
#define __DIRTREE_H

#include <istream>
#include <string>
#include <vector>
#include <filesystem>

#include "tree/tree.h"
#include "utils/subprocess.h"

namespace dirtree {

class DirTree : public treelib::Tree
{
public:
    std::filesystem::path path;
    DirTree(std::string _path);

    // Add the dir's contents (found with `find`), sorted by name, down to max_depth levels below
    // the dir (-1: all)
    void update_from_filesystem(int max_depth = -1);

    // find's error messages from the last update_from_filesystem(), one per path it couldn't
    // read (e.g. "find: '/a/b': Permission denied"), so the tree may lack what's under them
    std::vector<std::string> scan_errors;

    // Add the paths read from in, each ending with delimiter (or the end of input), in the order
    // they're read. Paths are relative to the tree's root, '/'-separated; see add_path().
    void update_from_stream(std::istream& in, char delimiter = '\n');

    // Add the node at path, after creating any of its ancestors that the tree doesn't have
    // yet. Empty and "." components are skipped (so "./a//b/" is "a/b"), and so are paths
    // already in the tree. The node's id is the cleaned path.
    void add_path(const std::string& path);

private:
    // Create the root node, unless there is one
    void add_root(const std::string& tag);
};

} // namespace dirtree

#endif
