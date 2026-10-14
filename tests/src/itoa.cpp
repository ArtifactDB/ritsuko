#include <gtest/gtest.h>

#include "ritsuko/itoa.hpp"

TEST(Itoa, Positive) {
    EXPECT_EQ(ritsuko::itoa(100), "100");
    EXPECT_EQ(ritsuko::itoa(98722332), "98722332");
    EXPECT_EQ(ritsuko::itoa(98722332u), "98722332");
    EXPECT_EQ(ritsuko::itoa(9), "9");
    EXPECT_EQ(ritsuko::itoa(1234567u), "1234567");
}

TEST(Itoa, Zero) {
    EXPECT_EQ(ritsuko::itoa(0), "0");
}

TEST(Itoa, Negative) {
    EXPECT_EQ(ritsuko::itoa(-100), "-100");
    EXPECT_EQ(ritsuko::itoa(-98722332), "-98722332");
    EXPECT_EQ(ritsuko::itoa(-9), "-9");
}
