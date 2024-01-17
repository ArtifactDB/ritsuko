#include "ritsuko/find_extremes.hpp"
#include <gtest/gtest.h>

TEST(FindExtremes, SignedInteger) {
    std::vector<int32_t> foo { 1, 2, 3 };
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end());
        EXPECT_FALSE(found.has_lowest);
        EXPECT_FALSE(found.has_highest);
        EXPECT_FALSE(found.has_zero);
    }

    foo.push_back(-2147483648);
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end());
        EXPECT_TRUE(found.has_lowest);
        EXPECT_FALSE(found.has_highest);
        EXPECT_FALSE(found.has_zero);
    }

    foo.push_back(2147483647);
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end());
        EXPECT_TRUE(found.has_lowest);
        EXPECT_TRUE(found.has_highest);
        EXPECT_FALSE(found.has_zero);
    }

    foo.push_back(0);
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end());
        EXPECT_TRUE(found.has_lowest);
        EXPECT_TRUE(found.has_highest);
        EXPECT_TRUE(found.has_zero);
    }
}

TEST(FindExtremes, UnsignedInteger) {
    std::vector<uint16_t> foo { 1, 2, 3 };
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end());
        EXPECT_FALSE(found.has_lowest);
        EXPECT_FALSE(found.has_highest);
        EXPECT_FALSE(found.has_zero);
    }

    foo.push_back(65535);
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end());
        EXPECT_FALSE(found.has_lowest);
        EXPECT_TRUE(found.has_highest);
        EXPECT_FALSE(found.has_zero);
    }

    foo.push_back(0);
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end());
        EXPECT_TRUE(found.has_lowest);
        EXPECT_TRUE(found.has_highest);
        EXPECT_TRUE(found.has_zero);
    }
}

TEST(FindExtremes, IntegerMask) {
    std::vector<int32_t> foo { 1, 2, 3 };
    std::vector<char> mask(foo.size());
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end(), mask.begin());
        EXPECT_FALSE(found.has_lowest);
        EXPECT_FALSE(found.has_highest);
        EXPECT_FALSE(found.has_zero);
    }

    foo.push_back(-2147483648);
    mask.push_back(0);
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end(), mask.begin());
        EXPECT_TRUE(found.has_lowest);
    }

    mask.back() = 1;
    {
        auto found = ritsuko::find_integer_extremes(foo.begin(), foo.end(), mask.begin());
        EXPECT_FALSE(found.has_lowest);
    }
}

TEST(FindExtremes, Float) {
    std::vector<double> foo { 1, 2, 3 };

    if constexpr(std::numeric_limits<double>::is_iec559) {
        {
            auto found = ritsuko::find_float_extremes(foo.begin(), foo.end());
            EXPECT_FALSE(found.has_nan);
        }
        auto nan = std::numeric_limits<double>::quiet_NaN();
        foo.push_back(nan);

        {
            auto found = ritsuko::find_float_extremes(foo.begin(), foo.end(), /* skip_nan = */ true);
            EXPECT_FALSE(found.has_nan);
            found = ritsuko::find_float_extremes(foo.begin(), foo.end());
            EXPECT_TRUE(found.has_nan);
            EXPECT_FALSE(found.has_positive_inf);
        }
        auto pinf = std::numeric_limits<double>::infinity();
        foo.push_back(pinf);

        {
            auto found = ritsuko::find_float_extremes(foo.begin(), foo.end());
            EXPECT_TRUE(found.has_positive_inf);
            EXPECT_FALSE(found.has_negative_inf);
        }
        auto ninf = -std::numeric_limits<double>::infinity();
        foo.push_back(ninf);

        {
            auto found = ritsuko::find_float_extremes(foo.begin(), foo.end());
            EXPECT_TRUE(found.has_negative_inf);
        }
    }

    {
        auto found = ritsuko::find_float_extremes(foo.begin(), foo.end());
        EXPECT_FALSE(found.has_lowest);
    }
    auto lowest = std::numeric_limits<double>::lowest();
    foo.push_back(lowest);

    {
        auto found = ritsuko::find_float_extremes(foo.begin(), foo.end());
        EXPECT_TRUE(found.has_lowest);
        EXPECT_FALSE(found.has_highest);
    }
    auto max = std::numeric_limits<double>::max();
    foo.push_back(max);

    {
        auto found = ritsuko::find_float_extremes(foo.begin(), foo.end());
        EXPECT_TRUE(found.has_highest);
        EXPECT_FALSE(found.has_zero);
    }
    foo.push_back(0);

    {
        auto found = ritsuko::find_float_extremes(foo.begin(), foo.end());
        EXPECT_TRUE(found.has_zero);
    }
}

TEST(FindExtremes, FloatMask) {
    std::vector<double> foo { 1, 2, 3 };
    std::vector<char> mask(foo.size());

    if constexpr(std::numeric_limits<double>::is_iec559) {
        auto nan = std::numeric_limits<double>::quiet_NaN();
        foo.push_back(nan);
        mask.push_back(0);
        {
            auto found = ritsuko::find_float_extremes(foo.begin(), foo.end(), mask.begin(), false);
            EXPECT_TRUE(found.has_nan);
            mask.back() = 1;
            found = ritsuko::find_float_extremes(foo.begin(), foo.end(), mask.begin(), false);
            EXPECT_FALSE(found.has_nan);
        }

        auto pinf = std::numeric_limits<double>::infinity();
        foo.push_back(pinf);
        mask.push_back(0);
        {
            auto found = ritsuko::find_float_extremes(foo.begin(), foo.end(), mask.begin(), false);
            EXPECT_TRUE(found.has_positive_inf);
            mask.back() = 1;
            found = ritsuko::find_float_extremes(foo.begin(), foo.end(), mask.begin(), false);
            EXPECT_FALSE(found.has_positive_inf);
        }
    }

    foo.push_back(0);
    mask.push_back(0);
    {
        auto found = ritsuko::find_float_extremes(foo.begin(), foo.end(), mask.begin(), false);
        EXPECT_TRUE(found.has_zero);
        mask.back() = 1;
        found = ritsuko::find_float_extremes(foo.begin(), foo.end(), mask.begin(), false);
        EXPECT_FALSE(found.has_zero);
    }
}
