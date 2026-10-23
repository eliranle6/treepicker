#include <sstream>
#include <string>
#include "gtest/gtest.h"

#include "picker/cli.h"
#include "picker/keys_help.h"

TEST(keys_help, fits_an_80_column_terminal) {
    // The picker shows it as is (the ? key), and the screen's last column stays empty
    std::istringstream keys(picker::KEYS_HELP);
    for (std::string line; std::getline(keys, line);) {
        ASSERT_LE(line.size(), 79) << line;
    }
}

TEST(keys_help, lists_the_help_key) {
    ASSERT_NE(std::string(picker::KEYS_HELP).find("  ?  "), std::string::npos);
}

TEST(keys_help, is_part_of_the_usage) {
    ASSERT_NE(picker::CLI_OPTIONS_USAGE.find(picker::KEYS_HELP), std::string::npos);
}
