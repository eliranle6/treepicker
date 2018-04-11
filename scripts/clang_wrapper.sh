# The purpose of this script is to wrap the clang compiler command so that it can be
# used by autotools in such a way that passes unique flag per file.
#
# Specifically, the "-MJ" flag of clang is needed to create the compilation database
# JSON file, but this flag requires an output filename, so it can't simply be put
# AM_CXXFLAGS in the Makefile.am (as an additional compilation flag), since a unique
# (per-file) flag cannot be specified in Makefile.am's AM_CXXFLAGS (for instance,
# specifying '-MJ bla.compilation-database.json' in the command that compiles bla.cpp).
# For this purpose, this script uses as a wrapper for the compiler; It intercepts the
# target object file and adds the "-MJ" flag with a json filename that is unique
# for the object file that is being built in that command.

# Find the last flag
for objectfile_name; do true; done

# Run the compilation with the compilation database flag
clang $@ -MJ $objectfile_name.compilation-database.json
#rc -c clang $@ -MJ $objectfile_name.compilation-database.json
#rc -J $objectfile_name.compilation-database.json
