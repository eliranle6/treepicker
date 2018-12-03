#ifndef __DIRPICKER_H
#define __DIRPICKER_H

#include "picker/picker.h"
#include "dirtree/dirtree.h"
#include <boost/filesystem/path.hpp>

namespace picker {
template class Picker<dirtree::FileEntry>;
}

namespace dirpicker {

using DirPicker = picker::Picker<dirtree::FileEntry>;

} // namespace dirpicker

#endif
