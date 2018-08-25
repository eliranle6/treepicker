#ifndef __DIRTREE_H
#define __DIRTREE_H

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>

#include "tree/tree.h"
#include "utils/subprocess.h"

namespace dirtree {

class FileEntry
{
};

}

// Template instantiation must be inside the namespace in which the template class
// was defined.
namespace treelib {
template class Tree<dirtree::FileEntry>;
}

namespace dirtree {

using FileEntryTree = treelib::Tree<FileEntry>;

class DirTree : public FileEntryTree
{
public:
    boost::filesystem::path path;
    DirTree(std::string _path);

    void update_from_filesystem(void);
};

} // namespace dirtree

#endif
