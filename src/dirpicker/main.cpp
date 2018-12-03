#include "picker/picker.h"
#include "dirpicker/dirpicker.h"

int main(void) {
    dirtree::DirTree tree(".");
    tree.update_from_filesystem();
    dirpicker::DirPicker picker(tree);
    picker.start();

    return 0;
}
