#include <algorithm>
#include <tuple>
#include "paginator.h"

std::tuple<int, int> paginate(int nr_items,
                              int page_size,
                              int selected_item_idx) {
    int nr_items_removed_at_the_beginning = 0;
    int nr_items_removed_at_the_end = 0;

    // A page needs room for the selection and a '... (N more)' row on each side of it. Smaller
    // windows get those 3 rows anyway, since hiding the selection would break navigation.
    page_size = std::max(page_size, 3);

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
        // Whatever comes after the selection takes one row: either the '... (N more)' row, or,
        // if only one item is hidden, that item instead (no point hiding one item behind a row)
        const bool is_row_used_after_selection = nr_items_removed_at_the_end > 0;
        if (nr_items_removed_at_the_end == 1) {
            nr_items_removed_at_the_end = 0;
        }

        nr_items_removed_at_the_beginning = selected_item_idx - page_size + 2 +
            is_row_used_after_selection;
        nr_items_removed_at_the_beginning = std::max(0, nr_items_removed_at_the_beginning);
        if (nr_items_removed_at_the_beginning == 1) {
            nr_items_removed_at_the_beginning = 0;
        }
    }

    return std::make_tuple(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end);
}
