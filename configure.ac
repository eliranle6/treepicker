# AC_* - macros for autoconf
# AM_* - macros for automake

AC_INIT([amtree_exmple], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Use a C++ compiler
AC_PROG_CXX

# Header file for preprocessor directives (alternative for specifying -D flags when compiling)
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
