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

TEST(ChooseMissingPlaceholder, IntegerMask) {
    std::vector<int32_t> foo { 1, 2, 3 };
    std::vector<char> mask(foo.size());
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end(), mask.begin());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, -2147483648);
    }

    foo.push_back(-2147483648);
    mask.push_back(0);
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end(), mask.begin());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, 2147483647);
    }

    mask.back() = 1;
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end(), mask.begin());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, -2147483648);
    }

    mask.back() = 0;
    foo.push_back(2147483647);
    foo.push_back(0);
    foo.push_back(-2147483647);
    mask.push_back(0);
    mask.push_back(0);
    mask.push_back(1);
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end(), mask.begin());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, -2147483647);
    }

    mask.back() = 0;
    {
        auto found = ritsuko::choose_missing_integer_placeholder(foo.begin(), foo.end(), mask.begin());
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, -2147483646);
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

            auto found2 = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end(), /* skip_nan = */ true);
            EXPECT_TRUE(found2.first);
            EXPECT_EQ(found2.second, pinf);
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

TEST(ChooseMissingPlaceholder, FloatMask) {
    std::vector<double> foo { 1, 2, 3 };
    std::vector<char> mask(foo.size());

    if constexpr(std::numeric_limits<double>::is_iec559) {
        auto nan = std::numeric_limits<double>::quiet_NaN();
        auto pinf = std::numeric_limits<double>::infinity();
        auto ninf = -std::numeric_limits<double>::infinity();

        {
            auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end(), mask.begin(), false);
            EXPECT_TRUE(found.first);
            EXPECT_TRUE(std::isnan(found.second));
        }
        foo.push_back(nan);
        mask.push_back(1);

        {
            auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end(), mask.begin(), false);
            EXPECT_TRUE(found.first);
            EXPECT_TRUE(std::isnan(found.second));
        }
        mask.back() = 0;
        foo.push_back(pinf);
        foo.push_back(ninf);
        mask.push_back(0);
        mask.push_back(0);
    }

    auto max = std::numeric_limits<double>::max();
    auto lowest = std::numeric_limits<double>::lowest();
    foo.push_back(max);
    foo.push_back(lowest);
    foo.push_back(0);
    mask.push_back(0);
    mask.push_back(0);
    mask.push_back(0);

    foo.push_back(lowest/2);
    mask.push_back(1);
    {
        auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end(), mask.begin(), false);
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, lowest / 2);
    }

    mask.back() = 0;
    {
        auto found = ritsuko::choose_missing_float_placeholder(foo.begin(), foo.end(), mask.begin(), false);
        EXPECT_TRUE(found.first);
        EXPECT_TRUE(found.second < lowest / 2);
        EXPECT_TRUE(found.second > lowest);
    }
}
