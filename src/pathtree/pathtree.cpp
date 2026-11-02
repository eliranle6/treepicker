#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>
#include "pathtree.h"

namespace pathtree {

PathTree::PathTree(std::string _path) : path(_path)
{
}

void PathTree::add_dir_root(void) {
    char* realpath_ret_value = realpath(path.c_str(), nullptr);
    if (realpath_ret_value == nullptr) {
        throw std::runtime_error("Cannot read directory '" + path.string() + "': " +
                                 strerror(errno));
    }
    std::string root_tag = std::string(basename(realpath_ret_value));
    free(realpath_ret_value);
    // "/" has no name after its last '/'
    if (root_tag.empty()) {
        root_tag = "/";
    }
    add_root(root_tag);
    get_root()->kind = 'd';
}

// Call visit(name, kind) for each dir ('d') and, unless is_dirs_only, each file ('f') in dir, not
// what symlinks point at (so not into a symlinked dir), nor other types. Returns 0, or the errno of
// why it couldn't be read (or not all of it: what was read of it is visited). readdir() rather than
// std::filesystem's iterator or a find: measured on 318k entries, 0.62s, against 1.7s and 1.06s.
template <typename Visit>
static int visit_dir(const std::string& dir, bool is_dirs_only, Visit visit) {
    DIR* stream = opendir(dir.c_str());
    if (stream == nullptr) {
        return errno;
    }
    errno = 0;
    while (const dirent* entry = readdir(stream)) {
        const char* name = entry->d_name;
        if (name[0] == '.' and (name[1] == '\0' or (name[1] == '.' and name[2] == '\0'))) {
            continue;
        }
        unsigned char type = entry->d_type;
        if (type == DT_UNKNOWN) {
            // Not every filesystem says
            struct stat status;
            if (fstatat(dirfd(stream), name, &status, AT_SYMLINK_NOFOLLOW) != 0) {
                // Gone since it was listed, or unreadable: skip it
                errno = 0;
                continue;
            }
            type = S_ISDIR(status.st_mode) ? DT_DIR : S_ISREG(status.st_mode) ? DT_REG : DT_LNK;
        }
        if (type == DT_DIR) {
            visit(name, 'd');
        } else if (type == DT_REG and not is_dirs_only) {
            visit(name, 'f');
        }
        errno = 0;
    }
    const int error = errno;
    closedir(stream);
    return error;
}

void PathTree::update_from_filesystem(int max_depth, bool is_dirs_only) {
    add_dir_root();
    scan_errors.clear();

    // Each entry's path and kind
    std::vector<std::pair<std::string, char>> entries;
    const std::string root = path.string();
    // The dirs left to read: each one's path and depth (the root's is 0, what's in it 1)
    std::vector<std::pair<std::string, int>> dirs = {{"", 0}};
    while (not dirs.empty()) {
        const std::string dir = std::move(dirs.back().first);
        const int depth = dirs.back().second;
        dirs.pop_back();
        if (max_depth >= 0 and depth >= max_depth) {
            continue;
        }
        const std::string full_path = dir.empty() ? root : root + "/" + dir;
        const int error = visit_dir(full_path, is_dirs_only, [&](const char* name, char kind) {
            std::string entry_path = dir.empty() ? name : dir + "/" + name;
            if (kind == 'd') {
                dirs.emplace_back(entry_path, depth + 1);
            }
            entries.emplace_back(std::move(entry_path), kind);
        });
        if (error != 0) {
            scan_errors.push_back("'" + full_path + "': " + strerror(error));
        }
    }

    // Show each dir's entries sorted by name (the tree keeps them in the order they're added).
    // Sorted paths also list each dir before what's in it, which create_node needs.
    std::sort(entries.begin(), entries.end());

    // Populate pathtree
    for (auto& path_and_kind : entries) {
        add_path(path_and_kind.first, path_and_kind.second);
    }
}

void PathTree::update_lazily_from_filesystem(int max_depth, bool is_dirs_only) {
    add_dir_root();
    _lazy_max_depth = max_depth;
    _is_lazy_dirs_only = is_dirs_only;
    get_root()->are_children_loaded = false;
}

bool PathTree::load_children(const node_t& node) {
    if (node->are_children_loaded) {
        return false;
    }
    node->are_children_loaded = true;
    if (_lazy_max_depth >= 0 and get_node_depth(node) >= _lazy_max_depth) {
        return false;
    }

    std::vector<std::pair<std::string, char>> entries;
    const auto dir = node == get_root() ? path : path / node->identifier;
    // A dir that can't be read (or fails midway) keeps what was read of it
    visit_dir(dir.string(), _is_lazy_dirs_only, [&entries](const char* name, char kind) {
        entries.emplace_back(name, kind);
    });

    // By name, like update_from_filesystem()
    std::sort(entries.begin(), entries.end());
    const std::string prefix = node == get_root() ? "" : node->identifier + "/";
    for (auto& name_and_kind : entries) {
        auto child = create_node(name_and_kind.first, prefix + name_and_kind.first,
                                 node->identifier);
        child->kind = name_and_kind.second;
        child->are_children_loaded = name_and_kind.second != 'd';
    }
    return not entries.empty();
}

void PathTree::update_from_stream(std::istream& in, char delimiter, bool is_typed,
                                  bool is_dirs_only) {
    add_root(".");
    std::string entry;
    while (getline(in, entry, delimiter)) {
        if (not is_typed) {
            add_path(entry);
        } else if (entry.size() > 2 and entry[1] == ' ' and
                   (entry[0] == 'd' or (entry[0] == 'f' and not is_dirs_only))) {
            add_path(entry.substr(2), entry[0]);
        }
    }
}

// path without empty and "." components
static std::string clean_path(const std::string& path) {
    std::string cleaned;
    cleaned.reserve(path.size());
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

void PathTree::add_path(const std::string& path, char kind) {
    const std::string cleaned = clean_path(path);
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

std::vector<std::string> PathTree::files_not_in(treelib::Tree& other) {
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

void PathTree::add_root(const std::string& tag) {
    if (get_root() == nullptr) {
        create_node(tag, "", "");
    }
}

} // namespace pathtree
