#ifndef __PAGINATOR_H
#define __PAGINATOR_H

std::tuple<int, int> paginate(int nr_items,
                              int max_nr_items_in_page,
                              int selected_item_index);

#endif
