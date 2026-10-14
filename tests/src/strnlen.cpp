#include <gtest/gtest.h>

#include "ritsuko/strnlen.hpp"

#include <string>
#include <vector>

TEST(Strnlen, Basic) {
    std::string foo = "asdasdasdasd";
    EXPECT_EQ(ritsuko::strnlen(foo.c_str(), 5), 5);
    EXPECT_EQ(ritsuko::strnlen(foo.c_str(), 50), foo.size());

    std::vector<char> bar{ 'a', '\0', '\0', '\0' };
    EXPECT_EQ(ritsuko::strnlen(bar.data(), 5), 1);
    EXPECT_EQ(ritsuko::strnlen(bar.data(), 0), 0);
}
