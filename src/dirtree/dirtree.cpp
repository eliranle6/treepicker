#include <stdlib.h>
#include <sys/wait.h>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "dirtree.h"
#include "utils/subprocess.h"

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
    // A named string, since path.string() returns a temporary, which the pointers would outlive
    const std::string path_string = path.string();
    char const *argv[] = {"/usr/bin/find", path_string.c_str(),
        "!", "-path", path_string.c_str(), "-printf", "%y %P\\0", nullptr};
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

    // Show each dir's entries sorted by name (the tree keeps them in the order they're added).
    // Sorted paths also list each dir before what's in it, which create_node needs.
    std::sort(entry_paths.begin(), entry_paths.end());

    // Populate dirtree. find's paths are relative and never end with '/', so the name is what
    // follows the last '/', and the parent's path (its id) what precedes it.
    for (auto& entry_path : entry_paths) {
        const auto last_slash = entry_path.rfind('/');
        if (last_slash == std::string::npos) {
            create_node(entry_path, entry_path, "", nullptr);
        } else {
            create_node(entry_path.substr(last_slash + 1), entry_path,
                        entry_path.substr(0, last_slash), nullptr);
        }
    }
}

} // namespace dirtree
