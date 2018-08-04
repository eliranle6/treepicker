#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <iostream>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include "subprocess.h"

namespace utils {

typedef struct {
    int read_fd;
    int write_fd;
} pipe_fds_t;


fds_istreamp subprocess_read_output(const char *filename, char const *argv[]) {

    // The following implementation causes a segfault
    //int proc_output_fd = utils::subprocess_get_fd(filename, argv);
    //fd_objp_t proc_output_fd_obj =
    //    std::make_unique<fd_obj_t>(proc_output_fd, boost::iostreams::never_close_handle);
    //fd_bufferp_t fd_stream_buffer = std::make_unique<fd_buffer_t>(*proc_output_fd_obj);
    //istreamp proc_output_stream = std::make_unique< std::istream > (&(*fd_stream_buffer));
    //return proc_output_stream;

    int proc_output_fd = utils::subprocess_get_fd(filename, argv);
    auto proc_output_fd_objp =
        std::make_unique<fd_obj_t>(proc_output_fd, boost::iostreams::never_close_handle);
    return std::make_unique<fds_istream> (*proc_output_fd_objp);
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
    int error = pipe2(&pipe_fds.read_fd, O_NONBLOCK);
    if (error) {
        std::cout << "Failed running child process (pipe failure " << error << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Fork in order to run the subprocess in the child process */
    pid_t pid = fork();
    if (pid == 0) {
        /* Child code */
        redirect_pipe_to_stdout(pipe_fds.write_fd);
        execv(path, (char * const *)argv);
        /* Exit since if we're here then the above execve failed. */
        std::cout << "Failed running child process '" << path << "' with args"
            << argv << std::endl;
        exit(EXIT_FAILURE);
    } else {
        /* Parent code */
        /* Wait for child process to finish */
        wait(nullptr);
    }

    /* Parent code */
    return pipe_fds.read_fd;
}

} // namespace util
