#include <sstream>
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
    std::string filename = path.filename().string();
    FileEntry root_dir;
    create_node(filename, "", "", root_dir);

    // Execute the `find` command to explore the dir contents
    char const *argv[] = {"/usr/bin/find", path.string().c_str(),
        "!", "-path", path.string().c_str(), "-printf", "%y %P\n", nullptr};
    auto entries_stream = utils::subprocess_read_output("/usr/bin/find", argv);

    // Populate dirtree by reading dir entries from command output stram
    while (true) {
        std::string entry_type, entry_path;
        getline(*entries_stream, entry_type, ' ');

        // An empty line means the output ended
        if (entry_type == "") {
            break;
        }

        getline(*entries_stream, entry_path, '\n');

        // Skip non-files and non-directories
        if (entry_type != "d" and entry_type != "f") {
            continue;
        }

        bpath entry_bpath(entry_path);
        FileEntry entry;
        create_node(entry_bpath.filename().string(),
                    entry_path,
                    entry_bpath.parent_path().string(),
                    entry);
    }
}

} // namespace dirtree
