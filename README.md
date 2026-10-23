# cpptree
A small and simple C++ tree library.

This is for C++ practice only, so obviously i'm going to use features that are not necessarily needed.
Each time a new feature is used for the first time, the text 'Today i learned: <feature name>" will appear as a comment in the code, near that feature and in the commit message.
The names of the learned features will be taken from: https://github.com/AnthonyCalandra/modern-cpp-features.

## How to Build
Needs a C++17 compiler, ncurses (`libncurses-dev`) and, for the tests, Google Test (`libgtest-dev`,
or `scripts/install-google-test.sh`).

```
$ make            # builds build/dirpicker and build/picker
$ make test       # builds and runs the tests
$ make install    # copies the programs to /usr/local/bin (or: make install PREFIX=~/.local)
$ make clean      # removes build/
```
