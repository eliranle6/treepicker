# Improvement ideas

Not implemented: simplifications found by reading the code (2026-10-24 review), roughly by impact.
A backlog to pick from.

## Concepts

1. **Navigate the tree, not the last screen.** `TreeSelector::move_to_next()`/`move_to_prev()`
   ask the printer which node was printed after/before the selection, so every move needs a fresh
   layout first (page up/down call `layout_tree()` after each step), and there are special cases
   for when the selection wasn't printed, or the previous node is the printed root. If next/previous
   were the next/previous node of the tree's visible preorder (what a full listing would show), the
   selector wouldn't depend on the printer at all, and the printer would only draw around the
   selection. Behavior changes only at the edges of depth-limited views (moving into a `(...)`
   node's children).

2. **Know files from dirs.** The tree only knows nodes with children from nodes without. Given the
   types (e.g. find's `%y` letters on stdin, `d path`/`f path`), empty dirs would show as dirs, and
   the status line could count picked files rather than nodes (dirdelta's improvement-ideas #25).

3. **Names.** `dirpicker` has become a generic tree picker (stdin trees, `--list` menus), and
   `DirTree`'s `add_path()`/`update_from_stream()` aren't about dirs either; something like
   `treepick` and `PathTree` would say what they are.

## Code

4. **Unused node fields.** `Node::parent_node` is never set or read, `Node::data` (a `void*`) is
   never read, and `Node::path` is built for every node (a string concatenation per node) but no
   longer read, since search matches names. Removing them saves memory and part of the load time
   (about 120k path strings for /usr/share).

5. **Dead code.** `src/dirpicker/dirpicker.{h,cpp}` hold an empty namespace; `Tree::children(id)` is
   used only by its own tests; `TreePrinter::print()`'s default arguments are used only by tests.
   The `picker` demo program could go too, or stay as the smallest example of the library.

6. **A simpler printer.** Printing is `treeprinter.cpp`, `treewindowfitter.cpp`, `paginator.cpp`
   and `printnode.cpp`, about 600 lines, with ~15 fields of state kept during its DFS (the first
   and last paginated siblings, the node printed before/after the selection, depth bookkeeping for
   the connector lines...). Building the list of lines to show once (the visible nodes down to the
   depth that fits, with the selection's siblings paginated), then drawing it, would remove most of
   that state, especially with #1.

7. **A key table.** `TreeKeyboardSelector::char_pressed()` is a long switch per mode, each case
   ending with `print_tree()`, and keys are listed twice (the `KEYCODE_*` enum, and curses' `KEY_*`
   codes in the same cases). A table of {keys → action, description}, from which `keys_help.cpp`'s
   text is also generated, would keep the keys and their help in sync.

8. **Styles apart from codes.** guishell's `Colors` enum mixes ANSI codes (`BLUE = 34`, written when
   curses isn't running) with curses color pair ids (`SELECTED_DIRECTORY = 101`), and each color's
   pair id is its ANSI code. Two ids once collided (a bug fixed in 8f02164). A style enum with
   separate ANSI and curses mappings would remove the coincidences.

9. **Fewer search flags.** Nodes have `is_matching_search`, `is_ancestor_of_matching_search` and
   `is_search_highlighted`, for two things: whether the node is shown, and whether it matches.

10. **Thin layers.** `KeyboardReactor` is a 10-line loop and `Picker` only wires it to
    `TreeKeyboardSelector`; one class would do.
