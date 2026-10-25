# cpptree

A C++ tree library and `treepick`, a fast terminal tree picker built on it (like fzf, but for
trees: it shows a tree, lets the user move around, search and pick, and prints what was picked).
It's the author's C++ practice project, and dirdelta's picker: dirdelta
(github.com/levioctl/dirdelta) has it as a git submodule at `lib/cpptree` and runs `treepick` as
a subprocess for every pick and menu. Speed with big trees (100k+ nodes) is the point: keep each
key press well under a few ms, and loading linear.

README.md has the usage; `treepick --help` lists the options and keys. Ideas not done yet:
`docs/improvement-ideas.md`.

## Building and testing

A plain Makefile, everything built into `build/` (C++17, ncurses; Google Test for the tests):

```
make            # build/treepick, build/dirdiff, build/picker (a demo), build/libcpptree.a
make test       # builds the programs and a binary per test, runs them all
make clean
make install PREFIX=~/.local
```

`make test` fails for a test binary that runs no tests (gtest passes those): that once hid that
no test was being linked in.

Tests are gtest files in `src/*/tests/test_*.cpp`, each its own binary. Some are randomized or
sweeping on purpose, and caught real bugs: the paginator is checked over all small sizes, and
`test_tree_selector`'s random walks check that the selection is always shown and the tree fits.
To check the UI itself, run the program in a detached tmux session (`tmux new-session -d -x 80 -y
20 ...`), send keys with `tmux send-keys`, and read the screen with `tmux capture-pane -p` (`-e`
to see the colors). The picker draws on `/dev/tty`, so it needs a (pseudo-)terminal.

## Structure

```
src/tree/         Tree, Node (children keep the order they're added), Search (smartcase,
                  matches names; hides non-matching nodes, or only highlights matches)
src/treeprinter/  TreePrinter (lays out and prints the part of the tree that fits around the
                  selection), treewindowfitter (how many levels fit), paginator (siblings
                  that don't fit become "... N more" lines), printnode (one line)
src/picker/       TreeSelector (the selection, and picked nodes: a dir is picked when all
                  under it is, partly picked when some is), TreeKeyboardSelector (keys,
                  modes, the screen's layout: header, tree, stats line, search line),
                  KeyboardReactor (the key loop), Picker, cli (the programs' shared options
                  and output), keys_help (the key list, for --help and the ? key, made from
                  TreeKeyboardSelector's key table)
src/utils/        guishell (curses on /dev/tty, colors, cutting lines by display width),
                  subprocess (running find)
src/pathtree/     PathTree: a tree of paths, from find (sorted) or from a stream (in order)
src/treepick/     the treepick program; src/picker/main.cpp is the demo program
src/dirdiff/      the dirdiff program: the files of a dir that another lacks (PathTree::files_not_in),
                  for dirdelta
```

`j`/`k` move through what's printed (the printer says which node was printed after or before the
selection), not through the tree's order: so the screen stays steady while moving in it, rather
than being redrawn around each new selection, which is hard to follow (the author's choice,
2026-10-24). Past the screen's edges they go on, like scrolling a list: `j` on the last line to the
node a bigger screen would show next, `k` on the first child to the printed root.

Every program feature goes in the shared core (`src/picker` and below), so all programs get it;
a program's `main.cpp` only builds its tree and calls `picker::pick_and_print()`.

## Conventions

- "Today i learned": the first time a modern C++ feature
  (https://github.com/AnthonyCalandra/modern-cpp-features) is used, mark it with a
  `// Today i learned: <feature>` comment and mention it in the commit message (see README.md).
  Several features came in without it in 2026-10; ask before adding the comments.
- The tree is agnostic of what it holds: it doesn't sort; whoever fills it decides the order
  (PathTree sorts find's output).
- No Boost: the standard library only.
- Don't push; commit locally. The working copy at /home/eliran/code/cpptree and dirdelta's
  `lib/cpptree` are separate clones: commit here, then move dirdelta's submodule to it (fetch
  from this repo in `lib/cpptree`, fast-forward, commit the pin in dirdelta).
