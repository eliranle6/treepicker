#ifndef __DIRTREE_H
#define __DIRTREE_H

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>

#include "tree/tree.h"
#include "utils/subprocess.h"

class FileEntry
{
};

template class treelib::Tree<FileEntry>;
using FileEntryTree = treelib::Tree<FileEntry>;

class DirTree : public FileEntryTree
{
public:
    boost::filesystem::path path;
    DirTree(std::string _path);

    void update_from_filesystem(void);
};

#endif
