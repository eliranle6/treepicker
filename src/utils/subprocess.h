#ifndef __SUBPROCESS_H
#define __SUBPROCESS_H

#include <string>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

namespace utils {


typedef boost::iostreams::file_descriptor_source fd_obj_t;
typedef boost::iostreams::stream<fd_obj_t> fds_istream;
typedef std::unique_ptr<fds_istream> fds_istreamp;

/* Run a subprocess and return a pointer to a boost istream to read its standard output.
 *
 * - This is similar to popen, except that it does not use the shell to execute processes.
 * - The reason that a pointer is used is because i cannot find how (it seems impossible)
 *   to copy/move an istream object.
 */
fds_istreamp subprocess_read_output(const char *filename, char const *argv[]);

/* Run a subprocess and return a file descriptor connected to its standard output.
 *
 * This is similar to popen, except that it does not use the shell to execute processes. */
int subprocess_get_fd(const char *filename, char const *argv[]);

} // namespace utils

#endif
