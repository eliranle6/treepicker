#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include <cerrno>

#include "subprocess.h"

namespace utils {

typedef struct {
    int read_fd;
    int write_fd;
} pipe_fds_t;


FdInputStream::Buffer::int_type FdInputStream::Buffer::underflow(void) {
    ssize_t nr_bytes;
    do {
        nr_bytes = ::read(_fd, _data, sizeof(_data));
    } while (nr_bytes < 0 and errno == EINTR);
    if (nr_bytes <= 0) {
        return traits_type::eof();
    }
    setg(_data, _data, _data + nr_bytes);
    return traits_type::to_int_type(_data[0]);
}

FdInputStream::FdInputStream(int fd) :
    // The buffer is a member, which isn't constructed yet when the base class is, so it's set
    // below
    std::istream(nullptr),
    _buffer(fd)
{
    rdbuf(&_buffer);
}

FdInputStream::~FdInputStream(void) {
    ::close(_buffer.fd());
}

fds_istreamp subprocess_read_output(const char *filename, char const *argv[]) {
    int proc_output_fd = utils::subprocess_get_fd(filename, argv);
    return std::make_unique<fds_istream>(proc_output_fd);
}

static void redirect_pipe_to_stdout(int fd) {
    /* Duplicate fd to the smallest closed descriptor (which will be stdout in the next statement) */
    close(STDOUT_FILENO);
    int error = dup(fd);
    if (error < 0) {
        std::cout << "Failed running child process (dup failure. errno: " << errno << ")" << std::endl;
        exit(EXIT_FAILURE);
    }
}

int subprocess_get_fd(char const * path, char const * argv[]) {
    /* Prepare file descriptors for shared pipe with subprocess */
    pipe_fds_t pipe_fds;
    /* A blocking pipe: with O_NONBLOCK, the child's writes fail with EAGAIN once the pipe
     * buffer fills up, which silently drops (or cuts mid-line) its output */
    int error = pipe(&pipe_fds.read_fd);
    if (error) {
        std::cout << "Failed running child process (pipe failure " << error << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Fork in order to run the subprocess in the child process */
    pid_t pid = fork();
    if (pid == 0) {
        /* Child code */
        close(pipe_fds.read_fd);
        redirect_pipe_to_stdout(pipe_fds.write_fd);
        close(pipe_fds.write_fd);
        execv(path, (char * const *)argv);
        /* Exit since if we're here then the above execve failed. */
        std::cout << "Failed running child process '" << path << "' with args"
            << argv << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Parent code */
    /* Don't wait for the child here: it blocks once the pipe is full until the output is read.
     * Close the write end so that reading gets EOF when the child exits. The caller reaps the
     * child (wait()) after reading its output. */
    close(pipe_fds.write_fd);
    return pipe_fds.read_fd;
}

} // namespace util
