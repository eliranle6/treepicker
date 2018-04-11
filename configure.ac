# AC_* - macros for autoconf
# AM_* - macros for automake

AC_INIT([amtree_exmple], [1.0])

# The following are arguments for automake when invoked for building Makefile files,
# and not(!!!) for the compiler.
# `foreign` means that this package does not follow the GNU standards with regard to file structure.
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Use a C++ compiler
AC_PROG_CXX

# Header file for preprocessor directives (alternative for specifying -D flags when compiling)
# The config.h file should be later included by the source code (if needed).
AC_CONFIG_HEADERS([config.h])

# Output makefiles
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])

# Dunno what this is, but the GNU manual says to specify this if any libraries are built
AC_PROG_RANLIB

# Support for unusual archivers
AM_PROG_AR

# Tell autoconf to generate the makefiles
AC_OUTPUT
