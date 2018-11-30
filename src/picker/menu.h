#ifndef MENU_H__
#define MENU_H__

#include "guishell/guishell.h"
#include "tree/tree.h"

class Menu
{
public:
    Menu(treelib::Tree<int> tree);

    void char_pressed(char c);

    void _print_tree(void);

private:
    treelib::Tree<int>& _tree;
    guishell::GuiShell& _out;
    std::string _search_keyword;
};

#endif
