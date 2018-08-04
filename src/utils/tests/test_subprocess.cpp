#include <cstring>
#include <string>
#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <gtest/gtest.h>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>

#include "utils/subprocess.h"

TEST(utils, subprocess_get_fd) {
    char buffer[100];
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
