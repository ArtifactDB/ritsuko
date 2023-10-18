#include "ritsuko/r_missing_value.hpp"
#include <gtest/gtest.h>
#include <cmath>

TEST(RMissingValue, Basic) {
    auto missing = ritsuko::r_missing_value();
    EXPECT_TRUE(std::isnan(missing));

    // Checking that the payload is different from a regular NaN.
    auto regular = std::numeric_limits<double>::quiet_NaN();
    EXPECT_FALSE(ritsuko::are_floats_identical(&missing, &regular));

    // Also different from normal numbers, obviously.
    double normal = 0;
    EXPECT_FALSE(ritsuko::are_floats_identical(&missing, &normal));
    
    // Confirming that match to a missing value.
    auto missing2 = ritsuko::r_missing_value();
    EXPECT_TRUE(ritsuko::are_floats_identical(&missing, &missing2));
}
