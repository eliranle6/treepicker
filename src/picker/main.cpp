#include <assert.h>
#include <signal.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>

#include "dirtree/dirtree.h"
#include "guishell/guishell.h"
#include "menu.h"

static Menu *menu = nullptr;

void trigger_from_stdin(boost::asio::io_service *io_service) {
    assert(io_service != NULL);
    char c;
    while ((c = getch()) != 'q') {
        io_service->dispatch([=]() { menu->char_pressed(c); });
    }
    kill(getpid(), SIGTERM);
}

int main(void) {
    treelib::Tree<int> tree;
    tree.create_node("root_tag", "root_id", "", 10);
    tree.create_node("first", "first", "root_id", 10);
    tree.create_node("second", "second", "root_id", 10);
    tree.create_node("first_in_first", "first_in_first", "first", 10);
    menu = new Menu(tree);

    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);


    // Handle keyboard events from a separate thread.
    // See https://stackoverflow.com/questions/10934444/asio-service-handler-for-stdin-keypress
    std::thread listen_thread(trigger_from_stdin, &io_service);
    io_service.run();

    listen_thread.join();
    endwin();

    delete menu;
    return 0;
}
