#include "picker/keys_help.h"

namespace picker {

const char KEYS_HELP[] =
    "  h/j/k/l, arrows  move\n"
    "  g/G, Home/End    first/last sibling\n"
    "  Ctrl-D/Ctrl-U    page down/up\n"
    "  /                search as you type (Enter keeps the filter, Esc drops it)\n"
    "  Ctrl-/           search when Enter is pressed\n"
    "  f                switch between showing only the matches and the\n"
    "                   whole tree with the matches in yellow\n"
    "  Space            pick or unpick (with -m)\n"
    "  Enter            pick (with -m: finish)\n"
    "  ?                show these keys\n"
    "  q, Ctrl-C        quit\n";

} // namespace picker
