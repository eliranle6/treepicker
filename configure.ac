# AC_* - macros for autoconf
# AM_* - macros for automake

AC_INIT([cpptree], [1.0])

# The following are arguments for automake when invoked for building Makefile files,
# and not(!!!) for the compiler.
# `foreign` means that this package does not follow the GNU standards with regard to file structure.
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign])

# Use a C++ compiler
AC_LANG(C)
AC_LANG(C++)
AC_PROG_CXX(clang_wrapper.sh)
AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/ax_cxx_compile_stdcxx.m4])
AX_CXX_COMPILE_STDCXX(11, noext, mandatory)

# Header file for preprocessor directives (alternative for specifying -D flags when compiling)
# The config.h file should be later included by the source code (if needed).
AC_CONFIG_HEADERS([config.h])

# Output makefiles
AC_CONFIG_FILES([
 Makefile
 src/Makefile

 src/utils/Makefile
 src/utils/tests/Makefile

 src/tree/Makefile
 src/tree/tests/Makefile

 src/dirtree/Makefile
 src/dirtree/tests/Makefile

 src/guishell/Makefile
 src/guishell/tests/Makefile

 src/picker/Makefile
 src/picker/tests/Makefile

 src/dirpicker/Makefile
])

# Dunno what this is, but the GNU manual says to specify this if any libraries are built
AC_PROG_RANLIB

# Support for unusual archivers
AM_PROG_AR

# Tell autoconf to generate the makefiles
AC_OUTPUT
