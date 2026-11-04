# treepicker
A small and simple C++ tree library.

This is for C++ practice only, so obviously i'm going to use features that are not necessarily needed.
Each time a new feature is used for the first time, the text 'Today i learned: <feature name>" will appear as a comment in the code, near that feature and in the commit message.
The names of the learned features will be taken from: https://github.com/AnthonyCalandra/modern-cpp-features.

## How to Build
Needs a C++17 compiler, ncurses (`libncurses-dev`) and, for the tests, Google Test (`libgtest-dev`,
or `scripts/install-google-test.sh`).

```
$ make            # builds build/treepicker, build/dirdiff and build/picker
$ make test       # builds and runs the tests
$ make install    # copies the programs to /usr/local/bin (or: make install PREFIX=~/.local)
$ make clean      # removes build/
```

## treepicker
A fast tree picker for the terminal, like fzf but for trees: it shows a tree, lets you move around
and search it, and prints what you picked.

```
$ treepicker [DIR]                   # pick under DIR (default: .); prints the picked path
$ treepicker --lazy --dirs-only /    # pick a dir, reading each only when it's shown
$ treepicker --exclude .git --exclude build DIR    # without those two (paths relative to DIR)
$ find . -name '*.mp3' | treepicker -        # pick from paths read from stdin
$ treepicker -m -0 --header 'Pick files' - < paths    # several nodes, '\0'-separated
$ printf 'Copy\nRemove\n' | treepicker --list          # a menu
```

A second tree can sit beside the one you pick from, to look through while picking, e.g. where the
picked files would go:

```
$ treepicker -m --typed --side other.txt --side-root other -    # other.txt: its paths, as --typed's
```

`Ctrl-W l` moves into it and `Ctrl-W h` (or Esc) back. `--side-start PATH` starts there, on PATH or
its closest ancestor; `--side-select FD` reads paths to select in it as the selection moves. `--side-key
KEY` makes Enter there finish picking, printing KEY and its selected node, `--side-expect KEYS` does so
for other keys too, and `--side-hint KEY TEXT` says what they do in the keys row.

It draws on the terminal (`/dev/tty`), so stdin and stdout are free for the tree and the result.
It exits with 0 if something was picked, and 1 if the user quit. `treepicker --help` lists the
options and keys. dirdelta (github.com/eliranle6/dirdelta) uses it for all of its pickers.

## dirdiff
Prints the files of one dir that another doesn't have, by path only (no content comparison):

```
$ dirdiff A B        # A's files (paths relative to A) that B lacks; -0 ends them with '\0'
```

It scans both with the same fast scan as treepicker; what couldn't be read goes to stderr.
dirdelta uses it to diff two local dirs.

