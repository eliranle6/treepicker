#include "utils/guishell.h"

int main(void) {
    guishell::GuiShell &out = guishell::GuiShell::get_instance();
    out << "yo yo " << guishell::Color(guishell::BLUE) << "This be in blue, homeboy" << guishell::Color(guishell::DEFAULT) << " and default again";
    out.refresh();
    sleep(2);
    return 0;
}
