#include <tuple>
#include "paginator.h"

std::tuple<int, int> paginate(int nr_items,
                              int max_nr_items_in_page,
                              int selected_item_index) {
    int nr_items_removed_at_the_beginning = 0;
    int nr_items_removed_at_the_end = 0;

    // Remove after selected, if too long
    if (nr_items > max_nr_items_in_page) {
        nr_items_removed_at_the_end = std::min(nr_items - selected_item_index - 1,
                                                   nr_items - max_nr_items_in_page);
        nr_items_removed_at_the_end = std::max(0, nr_items_removed_at_the_end);

        // Remove nodes before selected, if too long
        if (nr_items > max_nr_items_in_page) {
            nr_items_removed_at_the_beginning = selected_item_index - max_nr_items_in_page + 1;
            //if (nr_items_removed_at_the_end == 0) {
            //    nr_items_removed_at_the_beginning -= 1;
            //}
            nr_items_removed_at_the_beginning = std::max(nr_items_removed_at_the_beginning, 0);
            //if (nr_items_removed_at_the_beginning == 1) {
            //    nr_items_removed_at_the_beginning = 0;
            //}
            //if (nr_items_removed_at_the_end == 1) {
            //    nr_items_removed_at_the_end = 0;
            //}
        }
    }
    return std::make_tuple(nr_items_removed_at_the_beginning, nr_items_removed_at_the_end);
}
