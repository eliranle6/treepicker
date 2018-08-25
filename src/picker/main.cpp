#include <ncurses.h>
#include <assert.h>
#include <signal.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>

#include "dirtree/dirtree.h"
#include "guishell/guishell.h"

treelib::Tree<int> tree;
static guishell::GuiShell *out = nullptr;

void button_clicked(char c) {
    tree.get_root()->tag += c;
    clear();
    *out << tree;
    out->refresh();
}

void trigger_from_stdin(boost::asio::io_service *io_service) {
    assert(io_service != NULL);
    char c;
    while ((c = getch()) != 'q') {
        io_service->dispatch([=]() { button_clicked(c); });
    }
    kill(getpid(), SIGTERM);
}

int main(void) {
    out = &guishell::GuiShell::get_instance();
    boost::asio::io_service io_service;
    boost::asio::io_service::work work(io_service);

    tree.create_node("root_tag", "root_id", "", 10);
    tree.create_node("first", "first", "root_id", 10);
    tree.create_node("second", "second", "root_id", 10);
    tree.create_node("first_in_first", "first_in_first", "first", 10);

    std::thread listen_thread(trigger_from_stdin, &io_service);
    io_service.run();

    listen_thread.join();
    endwin();
    return 0;
}
