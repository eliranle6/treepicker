#include <stdlib.h>
#include <sys/wait.h>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <boost/filesystem/path.hpp>
#include "dirtree.h"
#include "utils/subprocess.h"

using bpath = boost::filesystem::path;

namespace dirtree {

DirTree::DirTree(std::string _path) : path(_path)
{
}

void DirTree::update_from_filesystem(void) {
    // Create root node
    char* realpath_ret_value = realpath(path.c_str(), nullptr);
    if (realpath_ret_value == nullptr) {
        throw std::runtime_error("Cannot read directory '" + path.string() + "': " +
                                 strerror(errno));
    }
    std::string root_tag = std::string(basename(realpath_ret_value));
    free(realpath_ret_value);
    create_node(root_tag, "", "", nullptr);

    // Execute the `find` command to explore the dir contents. Entries end with '\0' rather
    // than '\n', since file names may contain newlines.
    char const *argv[] = {"/usr/bin/find", path.string().c_str(),
        "!", "-path", path.string().c_str(), "-printf", "%y %P\\0", nullptr};
    auto entries_stream = utils::subprocess_read_output("/usr/bin/find", argv);

    // Read the dirs and files (skipping other types, e.g. symlinks)
    std::vector<std::string> entry_paths;
    std::string entry;
    while (getline(*entries_stream, entry, '\0')) {
        const bool is_dir_or_file = entry.size() > 2 and entry[1] == ' ' and
            (entry[0] == 'd' or entry[0] == 'f');
        if (is_dir_or_file) {
            entry_paths.push_back(entry.substr(2));
        }
    }

    // Reap the find process
    wait(nullptr);

    // Sorted paths list each dir before what's in it, and each dir's entries in the order that
    // Node::add_child keeps them, so adding them is quick
    std::sort(entry_paths.begin(), entry_paths.end());

    // Populate dirtree
    for (auto& entry_path : entry_paths) {
        bpath entry_bpath(entry_path);
        create_node(entry_bpath.filename().string(),
                    entry_path,
                    entry_bpath.parent_path().string(),
                    nullptr);
    }
}

} // namespace dirtree
