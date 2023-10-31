#include "ritsuko/choose_missing_placeholder.hpp"
#include <gtest/gtest.h>

TEST(ChooseMissingPlaceholder, SignedInteger) {
    std::vector<int32_t> foo { 1, 2, 3 };
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, -2147483648);
    }

    foo.push_back(-2147483648);
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, 2147483647);
    }

    foo.push_back(2147483647);
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, 0);
    }

    foo.push_back(0);
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, -2147483647);
    }

    {
        std::vector<int8_t> foo;
        for (int i = -128; i < 128; ++i) {
            foo.push_back(i);
        }
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end());
        EXPECT_FALSE(found.first);
    }
}

TEST(ChooseMissingPlaceholder, UnsignedInteger) {
    std::vector<uint16_t> foo { 1, 2, 3 };
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, 65535);
    }

    foo.push_back(65535);
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, 0);
    }

    foo.push_back(0);
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, 4);
    }

    {
        std::vector<uint8_t> foo;
        for (int i = 0; i < 256; ++i) {
            foo.push_back(i);
        }
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end());
        EXPECT_FALSE(found.first);
    }
}

TEST(ChooseMissingPlaceholder, Float) {
    std::vector<double> foo { 1, 2, 3 };

    if constexpr(std::numeric_limits<double>::is_iec559) {
        auto nan = std::numeric_limits<double>::quiet_NaN();
        auto pinf = std::numeric_limits<double>::infinity();
        auto ninf = -std::numeric_limits<double>::infinity();

        {
            auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end());
            EXPECT_TRUE(found.first);
            EXPECT_TRUE(std::isnan(found.second));
        }
        foo.push_back(nan);

        {
            auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end());
            EXPECT_TRUE(found.first);
            EXPECT_EQ(found.second, pinf);
        }
        foo.push_back(pinf);

        {
            auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end());
            EXPECT_TRUE(found.first);
            EXPECT_EQ(found.second, ninf);
        }
        foo.push_back(ninf);
    }

    auto max = std::numeric_limits<double>::max();
    auto lowest = std::numeric_limits<double>::lowest();

    {
        auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, lowest);
    }
    foo.push_back(lowest);

    {
        auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, max);
    }
    foo.push_back(max);

    {
        auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, 0);
    }
    foo.push_back(0);

    {
        auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, lowest / 2);
    }
}