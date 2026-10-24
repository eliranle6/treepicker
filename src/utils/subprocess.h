#ifndef __SUBPROCESS_H
#define __SUBPROCESS_H

#include <istream>
#include <memory>
#include <streambuf>
#include <string>

namespace utils {

/* A buffered input stream reading from a file descriptor, which it closes when destroyed */
class FdInputStream : public std::istream {
public:
    explicit FdInputStream(int fd);
    ~FdInputStream(void);

private:
    class Buffer : public std::streambuf {
    public:
        explicit Buffer(int fd) : _fd(fd) {}
        int fd(void) const { return _fd; }

    protected:
        /* Read the next chunk, when everything read so far was consumed */
        int_type underflow(void) override;

    private:
        int _fd;
        char _data[64 * 1024];
    };

    Buffer _buffer;
};

typedef FdInputStream fds_istream;
typedef std::unique_ptr<fds_istream> fds_istreamp;

/* Run a subprocess and return a pointer to an istream to read its standard output.
 *
 * - This is similar to popen, except that it does not use the shell to execute processes.
 * - The reason that a pointer is used is because i cannot find how (it seems impossible)
 *   to copy/move an istream object.
 */
fds_istreamp subprocess_read_output(const char *filename, char const *argv[], int stderr_fd = -1);

/* Run a subprocess and return a file descriptor connected to its standard output.
 *
 * This is similar to popen, except that it does not use the shell to execute processes.
 * With a stderr_fd, the subprocess's standard error goes to it (else it's this process's). */
int subprocess_get_fd(const char *filename, char const *argv[], int stderr_fd = -1);

} // namespace utils

#endif
