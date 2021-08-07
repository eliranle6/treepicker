#include <tuple>
#include "paginator.h"
#include <syslog.h>

std::tuple<int, int> paginate(int nr_items,
                              int page_size,
                              int selected_item_idx) {
    int nr_items_removed_at_the_beginning = 0;
    int nr_items_removed_at_the_end = 0;

    // Remove after selected, if too long
    if (nr_items > page_size) {
        // Make room in window for the indicator row at the beginning '... (N more)'
        // (remove another item from the end, since the window is arranged such that
        // the selection is at the bottom, which means that nodes below it are the
        // ones that are removed)
        if (selected_item_idx < page_size - 2) {
            nr_items_removed_at_the_end = nr_items - page_size + 1;
        } else {
            nr_items_removed_at_the_end = nr_items - selected_item_idx - 1;
        }
        if (nr_items_removed_at_the_end == 1) {
            nr_items_removed_at_the_end = 0;
        }

        nr_items_removed_at_the_beginning = selected_item_idx - page_size + 2 +
            (nr_items_removed_at_the_end == 1) + (nr_items_removed_at_the_end > 0);
        nr_items_removed_at_the_beginning = std::max(0, nr_items_removed_at_the_beginning);
        if (nr_items_removed_at_the_beginning == 1) {
            nr_items_removed_at_the_beginning = 0;
        }
    }

    return std::make_tuple(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end);
}
