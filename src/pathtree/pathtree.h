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
    // the dir (-1: all). With is_dirs_only, only its dirs.
    void update_from_filesystem(int max_depth = -1, bool is_dirs_only = false);

    // Add only the dir itself, and its contents later, a dir at a time: each dir's entries (sorted
    // by name, like update_from_filesystem()'s) when load_children() is called for it. So a huge
    // tree (e.g. "/") is shown at once, and only what's looked at is read. Down to max_depth
    // levels below the dir (-1: all); with is_dirs_only, only its dirs.
    void update_lazily_from_filesystem(int max_depth = -1, bool is_dirs_only = false);

    // With update_lazily_from_filesystem(): add the dir's entries, if they aren't yet. A dir that
    // can't be read is left empty.
    bool load_children(const node_t& node) override;

    // find's error messages from the last update_from_filesystem(), one per path it couldn't
    // read (e.g. "find: '/a/b': Permission denied"), so the tree may lack what's under them
    std::vector<std::string> scan_errors;

    // Add the paths read from in, each ending with delimiter (or the end of input), in the order
    // they're read. Paths are relative to the tree's root, '/'-separated; see add_path().
    // With is_typed, each starts with its type and a space, like find's %y: 'f' a file, 'd' a dir
    // (others are skipped, like symlinks in a dir's scan); with is_dirs_only, files are skipped
    // too.
    void update_from_stream(std::istream& in, char delimiter = '\n', bool is_typed = false,
                            bool is_dirs_only = false);

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

    // Create the root node of the dir at path, named like it (a dir), unless there is one
    void add_dir_root(void);

    // What load_children() loads: down to how many levels below the root (-1: all), and whether
    // only dirs
    int _lazy_max_depth = -1;
    bool _is_lazy_dirs_only = false;
};

} // namespace pathtree

#endif
