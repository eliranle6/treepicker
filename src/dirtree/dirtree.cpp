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
    add_root(root_tag);

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

    // Populate dirtree
    for (auto& entry_path : entry_paths) {
        add_path(entry_path);
    }
}

void DirTree::update_from_stream(std::istream& in, char delimiter) {
    add_root(".");
    std::string entry_path;
    while (getline(in, entry_path, delimiter)) {
        add_path(entry_path);
    }
}

// Whether path has empty or "." components (find's paths never do)
static bool needs_cleaning(const std::string& path) {
    std::size_t component_start = 0;
    for (std::size_t i = 0; i <= path.size(); ++i) {
        if (i == path.size() or path[i] == '/') {
            const std::size_t length = i - component_start;
            if (length == 0 or (length == 1 and path[component_start] == '.')) {
                return true;
            }
            component_start = i + 1;
        }
    }
    return false;
}

// path without empty and "." components
static std::string clean_path(const std::string& path) {
    std::string cleaned;
    std::size_t component_start = 0;
    for (std::size_t i = 0; i <= path.size(); ++i) {
        if (i == path.size() or path[i] == '/') {
            const std::size_t length = i - component_start;
            if (length > 0 and not (length == 1 and path[component_start] == '.')) {
                if (not cleaned.empty()) {
                    cleaned += '/';
                }
                cleaned.append(path, component_start, length);
            }
            component_start = i + 1;
        }
    }
    return cleaned;
}

void DirTree::add_path(const std::string& path) {
    const std::string cleaned = needs_cleaning(path) ? clean_path(path) : path;
    if (cleaned.empty()) {
        return;
    }

    // The name is what follows the last '/', and the parent's path (its id) what precedes it
    const auto last_slash = cleaned.rfind('/');
    const std::string parent = last_slash == std::string::npos ? "" : cleaned.substr(0, last_slash);
    const std::string name = last_slash == std::string::npos ? cleaned : cleaned.substr(last_slash + 1);
    if (get_or_create_node(name, cleaned, parent, nullptr) == nullptr) {
        // The parent isn't in the tree yet
        add_path(parent);
        create_node(name, cleaned, parent, nullptr);
    }
}

void DirTree::add_root(const std::string& tag) {
    if (get_root() == nullptr) {
        create_node(tag, "", "", nullptr);
    }
}

} // namespace dirtree
