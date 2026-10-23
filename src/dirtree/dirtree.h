#ifndef __DIRTREE_H
#define __DIRTREE_H

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

    void update_from_filesystem(void);
};

} // namespace dirtree

#endif
