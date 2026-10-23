#include <cstring>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <gtest/gtest.h>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include "utils/subprocess.h"

TEST(utils, subprocess_get_fd) {
    char buffer[100] = {};
    char const * command[] = {"/bin/echo", "foobar", nullptr};
    int fd = utils::subprocess_get_fd("/bin/echo", command);
    size_t nr_bytes = read(fd, &buffer[0], sizeof(buffer));
    (void)nr_bytes;
    ASSERT_STREQ(buffer, "foobar\n");
}

TEST(utils, subprocess_read_output) {
    char const * command[] = {"/bin/echo", "firstline\n\nsecondline\nthirdline", nullptr};
    auto subprocess_output_streamp = utils::subprocess_read_output("/bin/echo", command);

    std::string line;

    *subprocess_output_streamp >> line;
    ASSERT_STREQ(line.c_str(), "firstline");
    *subprocess_output_streamp >> line;
    ASSERT_STREQ(line.c_str(), "secondline");
    *subprocess_output_streamp >> line;
    ASSERT_STREQ(line.c_str(), "thirdline");
}

TEST(utils, subprocess_read_output_bigger_than_the_pipe_buffer) {
    // A pipe holds 64KB. The output was once lost (or cut mid-line) past that, since the child
    // wrote to a non-blocking pipe that nobody read until the child exited.
    char const * command[] = {"/usr/bin/seq", "1", "200000", nullptr};
    auto subprocess_output_streamp = utils::subprocess_read_output("/usr/bin/seq", command);

    std::string line;
    int nr_lines = 0;
    while (std::getline(*subprocess_output_streamp, line)) {
        ++nr_lines;
        ASSERT_EQ(line, std::to_string(nr_lines));
    }
    ASSERT_EQ(nr_lines, 200000);
    wait(nullptr);
}

TEST(utils, subprocess_output_ends) {
    // Reading ends once the child exits (the parent doesn't keep the pipe's write end open)
    char const * command[] = {"/bin/echo", "only line", nullptr};
    auto subprocess_output_streamp = utils::subprocess_read_output("/bin/echo", command);
    std::string line;
    ASSERT_TRUE(std::getline(*subprocess_output_streamp, line));
    ASSERT_EQ(line, "only line");
    ASSERT_FALSE(std::getline(*subprocess_output_streamp, line));
    wait(nullptr);
}
