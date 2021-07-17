#include <locale.h>
#include "picker/picker.h"
#include "dirtree/dirtree.h"

int main(void) {
    setlocale(LC_ALL, "");
    dirtree::DirTree tree(".");
    tree.update_from_filesystem();
    picker::Picker picker(tree);
    picker.run();

    return 0;
}
