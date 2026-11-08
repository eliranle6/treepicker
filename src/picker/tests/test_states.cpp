#include <random>
#include "gtest/gtest.h"

#include "picker/states.h"
#include "picker/tabs.h"
#include "tree/search.h"
#include "treeprinter/treeprinter.h"

// A tree of two dirs, a/ (x, y) and b/ (z), its files in groups "A" (0) and "B" (1)
static treelib::Tree get_tree(void) {
    treelib::Tree tree;
    tree.create_node(".", ".", "");
    tree.create_node("a", "a", ".");
    tree.create_node("x", "a/x", "a");
    tree.create_node("y", "a/y", "a");
    tree.create_node("b", "b", ".");
    tree.create_node("z", "b/z", "b");
    return tree;
}

// States skip, to-b (key '>', shown as '+' for group B) and to-a ('<'); x and z start skipped, y
// in to-b; z allows only skip and to-a
static picker::PickerOptions get_options(void) {
    picker::PickerOptions options;
    options.is_multi = true;
    options.groups = {"A", "B"};
    options.states = {{"skip", " ", "grey", 0, {}},
                      {"to-b", ">", "green", '>', {{1, "x", "red"}}},
                      {"to-a", "<", "green", '<', {}}};
    options.node_states = {{"a/x", ~0u, 0, 0}, {"a/y", ~0u, 1, 0}, {"b/z", 0b101, 0, 1}};
    return options;
}

struct StatesTest : ::testing::Test {
    treelib::Tree tree = get_tree();
    treelib::TreePrinter printer{tree};
    picker::TreeSelector selector{tree, printer};
    picker::States states{tree, selector, get_options()};

    int state(const std::string& id) { return tree.get_node(id)->state; }
};

TEST_F(StatesTest, start_in_their_states_and_dirs_follow) {
    ASSERT_EQ(state("a/x"), 0);
    ASSERT_EQ(state("a/y"), 1);
    ASSERT_EQ(state("a"), treelib::Node::MIXED_STATE);
    ASSERT_EQ(state("b"), 0);
    // A dir's group is its nodes', if they're all in one
    ASSERT_EQ(tree.get_node("a")->group, 0);
    ASSERT_EQ(tree.get_node("b")->group, 1);
    ASSERT_EQ(tree.get_root()->group, -1);
    // Picked: the nodes not in the first state
    ASSERT_EQ(selector.get_nr_picked(), 1u);
    ASSERT_TRUE(tree.get_node("a/y")->is_picked);
}

TEST_F(StatesTest, set_on_a_dir_sets_what_allows_it) {
    states.set(tree.get_root(), 1, false);
    ASSERT_EQ(state("a/x"), 1);
    ASSERT_EQ(state("a/y"), 1);
    ASSERT_EQ(state("a"), 1);
    // z doesn't allow it
    ASSERT_EQ(state("b/z"), 0);
    ASSERT_EQ(state("."), treelib::Node::MIXED_STATE);
    states.set(tree.get_root(), 2, false);
    ASSERT_EQ(state("."), 2);
    ASSERT_EQ(states.get_counts_text(), "< 3");
}

TEST_F(StatesTest, cycle_goes_through_what_all_allow) {
    // b/z: skip, then to-a (it doesn't allow to-b), then skip again
    states.cycle(tree.get_node("b"), false);
    ASSERT_EQ(state("b/z"), 2);
    states.cycle(tree.get_node("b"), false);
    ASSERT_EQ(state("b/z"), 0);
    // The root: what all allow (skip and to-a), from skip (it's mixed)
    states.cycle(tree.get_root(), false);
    ASSERT_EQ(state("."), 2);
}

TEST_F(StatesTest, only_shown_leaves_the_hidden_ones) {
    treelib::Search search(tree);
    search.set_filter([](const treelib::Node& node) { return node.group == 0; });
    search.search("");
    states.set(tree.get_root(), 2, true);
    ASSERT_EQ(state("a/x"), 2);
    ASSERT_EQ(state("a/y"), 2);
    ASSERT_EQ(state("b/z"), 0);
}

TEST_F(StatesTest, counts_by_mark_and_reset) {
    states.set(tree.get_node("a/x"), 1, false);
    states.set(tree.get_node("b/z"), 2, false);
    ASSERT_EQ(states.get_counts_text(), "> 2  < 1");
    states.reset();
    ASSERT_EQ(states.get_counts_text(), "");
    ASSERT_EQ(selector.get_nr_picked(), 0u);
    ASSERT_EQ(state("."), 0);
}

TEST_F(StatesTest, keys) {
    ASSERT_EQ(states.get_state_of_key('>'), 1);
    ASSERT_EQ(states.get_state_of_key('<'), 2);
    ASSERT_EQ(states.get_state_of_key('q'), -1);
}

TEST_F(StatesTest, marks_by_group) {
    auto marks = states.get_marks();
    ASSERT_NE(marks, nullptr);
    ASSERT_EQ((*marks)[1][0].mark, ">");
    // Group B (1) shows to-b as 'x'
    ASSERT_EQ((*marks)[1][2].mark, "x");
}

TEST(states, off_without_states) {
    auto tree = get_tree();
    treelib::TreePrinter printer(tree);
    picker::TreeSelector selector(tree, printer);
    picker::States states(tree, selector, picker::PickerOptions());
    ASSERT_FALSE(states.is_on());
    ASSERT_EQ(states.get_marks(), nullptr);
    ASSERT_EQ(tree.get_node("a/x")->state, treelib::Node::NO_STATE);
}

// Random settings never leave a dir's state disagreeing with its nodes', nor the picked count
// with the nodes not in the first state
TEST(states, dirs_always_agree_with_their_nodes) {
    treelib::Tree tree;
    tree.create_node(".", ".", "");
    std::vector<std::string> ids = {"."};
    std::mt19937 random(7);
    for (int i = 0; i < 60; ++i) {
        const std::string parent = ids[random() % ids.size()];
        const std::string id = parent + "/" + std::to_string(i);
        tree.create_node(std::to_string(i), id, parent);
        ids.push_back(id);
    }
    auto options = get_options();
    options.node_states.clear();
    for (auto& id : ids) {
        options.node_states.push_back({id, random() % 2 ? ~0u : 0b011u, 0, static_cast<int>(random() % 2)});
    }
    treelib::TreePrinter printer(tree);
    picker::TreeSelector selector(tree, printer);
    picker::States states(tree, selector, options);
    treelib::Search search(tree);
    for (int step = 0; step < 300; ++step) {
        if (step % 50 == 0) {
            const int group = step / 50 % 3 - 1;
            search.set_filter(group < 0 ? std::function<bool(const treelib::Node&)>() :
                              [group](const treelib::Node& node) { return node.group == group; });
            search.search("");
        }
        auto node = tree.get_node(ids[random() % ids.size()]);
        if (random() % 2) {
            states.cycle(node, step % 100 < 50);
        } else {
            states.set(node, random() % 3, step % 100 < 50);
        }
        std::size_t nr_set = 0;
        for (auto& id : ids) {
            auto current = tree.get_node(id);
            if (current->children.empty()) {
                ASSERT_GE(current->state, 0);
                ASSERT_TRUE(current->allowed_states & (1u << current->state));
                nr_set += current->state != 0;
                ASSERT_EQ(current->is_picked, current->state != 0);
                continue;
            }
            int expected = current->children.front()->state;
            for (auto& child : current->children) {
                if (child->state != expected) {
                    expected = treelib::Node::MIXED_STATE;
                }
            }
            ASSERT_EQ(current->state, expected) << id;
        }
        ASSERT_EQ(states.get_set_nodes().size(), nr_set);
    }
}

TEST(tabs, count_switch_and_filter) {
    auto tree = get_tree();
    auto options = get_options();
    options.tabs = {{"All", {}}, {"Only A", {0}}, {"Only B", {1}}};
    tree.get_node("a/x")->group = 0;
    tree.get_node("a/y")->group = 0;
    tree.get_node("b/z")->group = 1;
    picker::Tabs tabs(tree, options);
    ASSERT_TRUE(tabs.is_on());
    ASSERT_EQ(tabs.get_titles(), (std::vector<std::string>{"All 3", "Only A 2", "Only B 1"}));
    ASSERT_FALSE(tabs.is_filtering());
    ASSERT_EQ(tabs.get_filter(), nullptr);
    tabs.move(-1);
    ASSERT_EQ(tabs.get_current(), 2u);
    ASSERT_TRUE(tabs.is_filtering());
    ASSERT_TRUE(tabs.get_filter()(*tree.get_node("b/z")));
    ASSERT_FALSE(tabs.get_filter()(*tree.get_node("a/x")));
    tabs.move(1);
    ASSERT_EQ(tabs.get_current(), 0u);
}

TEST_F(StatesTest, a_state_is_found_by_its_name) {
    ASSERT_EQ(states.get_state_named("to-a"), 2);
    ASSERT_EQ(states.get_state_named("nothing"), -1);
}
