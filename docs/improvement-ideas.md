# Improvement ideas

Not implemented: what's left of the 2026-10-24 review's simplifications, and ideas found since,
roughly by impact. A backlog to pick from; the done ones are in git history.

## Concepts

1. **`j` into a collapsed dir.** `j`/`k` move through what's printed (see CLAUDE.md), and scroll
   past the screen's edges, but in a wide view (`music >`, its artists shown as `Bach (...)`),
   `j` goes from artist to artist and never into one: `l` does that. It could, at the last one,
   go on into the first artist's albums, as a full listing would; or stay as it is, since `l`
   is right there.

2. **Names.** `dirpicker` has become a generic tree picker (stdin trees, `--list` menus), and
   `DirTree`'s `add_path()`/`update_from_stream()` aren't about dirs either; something like
   `treepick` and `PathTree` would say what they are.

## Code

3. **A simpler printer.** Printing is `treeprinter.cpp`, `treewindowfitter.cpp`, `paginator.cpp`
   and `printnode.cpp`, about 600 lines, with ~15 fields of state kept during its DFS (the first
   and last paginated siblings, the node printed before/after the selection, depth bookkeeping for
   the connector lines...). Building the list of lines to show once (the visible nodes down to the
   depth that fits, with the selection's siblings paginated), then drawing it, would remove most of
   that state.

4. **Thin layers.** `KeyboardReactor` is a 10-line loop and `Picker` only wires it to
   `TreeKeyboardSelector`; one class would do.
