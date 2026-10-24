#include <stdlib.h>
#include <sys/wait.h>
#include <algorithm>
#include <cerrno>
#include <cstdio>
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

void DirTree::update_from_filesystem(int max_depth) {
    // Create root node
    char* realpath_ret_value = realpath(path.c_str(), nullptr);
    if (realpath_ret_value == nullptr) {
        throw std::runtime_error("Cannot read directory '" + path.string() + "': " +
                                 strerror(errno));
    }
    std::string root_tag = std::string(basename(realpath_ret_value));
    free(realpath_ret_value);
    add_root(root_tag);
    get_root()->kind = 'd';

    // Execute the `find` command to explore the dir contents. Entries end with '\0' rather
    // than '\n', since file names may contain newlines.
    // A named string, since path.string() returns a temporary, which the pointers would outlive
    const std::string path_string = path.string();
    const std::string max_depth_string = std::to_string(max_depth);
    std::vector<char const*> argv = {"/usr/bin/find", path_string.c_str()};
    if (max_depth >= 0) {
        argv.insert(argv.end(), {"-maxdepth", max_depth_string.c_str()});
    }
    argv.insert(argv.end(), {"!", "-path", path_string.c_str(), "-printf", "%y %P\\0", nullptr});
    // find's errors go to a temporary file rather than a pipe: reading two pipes one after the
    // other could block find on a full one
    FILE* errors_file = std::tmpfile();
    auto entries_stream = utils::subprocess_read_output(
        "/usr/bin/find", argv.data(), errors_file == nullptr ? -1 : fileno(errors_file));

    // Read the dirs and files (skipping other types, e.g. symlinks)
    // Each its path and kind
    std::vector<std::pair<std::string, char>> entries;
    std::string entry;
    while (getline(*entries_stream, entry, '\0')) {
        const bool is_dir_or_file = entry.size() > 2 and entry[1] == ' ' and
            (entry[0] == 'd' or entry[0] == 'f');
        if (is_dir_or_file) {
            entries.emplace_back(entry.substr(2), entry[0]);
        }
    }

    // Reap the find process
    wait(nullptr);

    scan_errors.clear();
    if (errors_file != nullptr) {
        std::rewind(errors_file);
        char* line = nullptr;
        std::size_t line_capacity = 0;
        ssize_t line_size;
        while ((line_size = getline(&line, &line_capacity, errors_file)) > 0) {
            scan_errors.emplace_back(line, line[line_size - 1] == '\n' ? line_size - 1 : line_size);
        }
        free(line);
        std::fclose(errors_file);
    }

    // Show each dir's entries sorted by name (the tree keeps them in the order they're added).
    // Sorted paths also list each dir before what's in it, which create_node needs.
    std::sort(entries.begin(), entries.end());

    // Populate dirtree
    for (auto& path_and_kind : entries) {
        add_path(path_and_kind.first, path_and_kind.second);
    }
}

void DirTree::update_from_stream(std::istream& in, char delimiter, bool is_typed) {
    add_root(".");
    std::string entry;
    while (getline(in, entry, delimiter)) {
        if (not is_typed) {
            add_path(entry);
        } else if (entry.size() > 2 and entry[1] == ' ' and (entry[0] == 'f' or entry[0] == 'd')) {
            add_path(entry.substr(2), entry[0]);
        }
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

void DirTree::add_path(const std::string& path, char kind) {
    const std::string cleaned = needs_cleaning(path) ? clean_path(path) : path;
    if (cleaned.empty()) {
        return;
    }

    // The name is what follows the last '/', and the parent's path (its id) what precedes it
    const auto last_slash = cleaned.rfind('/');
    const std::string parent = last_slash == std::string::npos ? "" : cleaned.substr(0, last_slash);
    const std::string name = last_slash == std::string::npos ? cleaned : cleaned.substr(last_slash + 1);
    auto node = get_or_create_node(name, cleaned, parent);
    if (node == nullptr) {
        // The parent isn't in the tree yet
        add_path(parent, 'd');
        node = create_node(name, cleaned, parent);
    }
    if (kind != 0) {
        node->kind = kind;
    }
}

std::vector<std::string> DirTree::files_not_in(treelib::Tree& other) {
    std::vector<std::string> missing;
    if (get_root() == nullptr) {
        return missing;
    }
    std::vector<treelib::Node*> dfs_stack = {get_root().get()};
    while (not dfs_stack.empty()) {
        auto node = dfs_stack.back();
        dfs_stack.pop_back();
        if (node->kind == 'f' and other.get_node(node->identifier) == nullptr) {
            missing.push_back(node->identifier);
        }
        // Reversed, so the first child is popped first
        for (auto child = node->children.rbegin(); child != node->children.rend(); ++child) {
            dfs_stack.push_back(child->get());
        }
    }
    return missing;
}

void DirTree::add_root(const std::string& tag) {
    if (get_root() == nullptr) {
        create_node(tag, "", "");
    }
}

} // namespace dirtree
