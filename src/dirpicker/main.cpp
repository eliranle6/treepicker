#include <locale.h>
#include "picker/picker.h"
#include "dirpicker/dirpicker.h"

int main(void) {
    setlocale(LC_ALL, "");
    dirtree::DirTree tree(".");
    tree.update_from_filesystem();
    dirpicker::DirPicker picker(tree);
    picker.run();

    return 0;
}
