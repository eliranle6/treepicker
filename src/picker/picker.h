#ifndef __PICKER_H
#define __PICKER_H

#include <assert.h>
#include <signal.h>
#include <thread>
#include <boost/asio.hpp>
#include "menu.h"
#include "tree/tree.h"
#include "guishell/guishell.h"

namespace picker {

template<typename T>
class Picker {
public:
    Picker(treelib::Tree<T>& tree);

    void start(void);

    void trigger_from_stdin(boost::asio::io_service *io_service);

private:
    boost::asio::io_service _io_service;
    Menu<T> _menu;
    treelib::Tree<T>& _tree;
};

template<typename T>
Picker<T>::Picker(treelib::Tree<T>& tree) :
    _menu(tree),
    _tree(tree)
{
}

template<typename T>
void Picker<T>::start(void) {
    boost::asio::io_service::work work(_io_service);

    guishell::GuiShell::get_instance() << _tree;
    // Handle keyboard events from a separate thread.
    // See https://stackoverflow.com/questions/10934444/asio-service-handler-for-stdin-keypress
    std::thread listen_thread(&Picker<T>::trigger_from_stdin, this, &_io_service);
    _io_service.run();

    listen_thread.join();
    endwin();
}

template<typename T>
void Picker<T>::trigger_from_stdin(boost::asio::io_service *io_service) {
    assert(io_service != NULL);
    char c;
    while ((c = getch()) != 'q') {
        _io_service.dispatch([=]() { _menu.char_pressed(c); });
    }
    kill(getpid(), SIGTERM);
}

} // namespace picker

#endif
