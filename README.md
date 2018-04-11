# cpptree
A small and simple C++ tree library.

## How to Build
This package uses autotools as the build system. I chose it only to practice it and not because i think it's a good choice.

1. Build the build system:
```
$ autoreconf --install
```
This will, among other things:
* Call autoconf to generate the 'configure' script from configure.ac
* Call automake to generate Makefile.in from Makefile.am
(See https://www.gnu.org/software/autoconf/manual/autoconf-2.68/html_node/autoreconf-Invocation.html)

2. Configure (This generates Makefile files from the Makefile.in files):
```
$ ./configure
```
3. Build:
```
$ make
```

