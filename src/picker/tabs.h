#ifndef PICKER_TABS_H__
#define PICKER_TABS_H__

#include <functional>
#include <string>
#include <vector>

#include "picker/picker_options.h"
#include "tree/tree.h"

namespace picker {

// The tabs above the tree (PickerOptions::tabs), each showing some groups of the nodes (by
// Node::group), and which one is shown
class Tabs
{
public:
    // Count each tab's nodes (the ones without children), once: the groups don't change
    Tabs(treelib::Tree& tree, const PickerOptions& options);

    bool is_on(void) const { return not _tabs.empty(); }

    // Show the next tab (step 1) or the previous one (-1), going round
    void move(int step);
    std::size_t get_current(void) const { return _current; }

    // Whether the tab shown leaves nodes out
    bool is_filtering(void) const;
    // The filter of the nodes without children it shows (see Search::set_filter()); nullptr if it
    // shows them all
    std::function<bool(const treelib::Node&)> get_filter(void) const;

    // Each tab's title and how many nodes it shows: "Only in A 120"
    std::vector<std::string> get_titles(void) const;

private:
    std::vector<TabOption> _tabs;
    std::vector<std::size_t> _counts;
    std::size_t _current = 0;
};

} // namespace picker

#endif
