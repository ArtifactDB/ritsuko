#include "ritsuko/is_date_time.hpp"
#include <gtest/gtest.h>
#include <cmath>

bool is_date(std::string x) {
    return ritsuko::is_date(x.c_str(), x.size());
}

bool is_rfc3339(std::string x) {
    return ritsuko::is_rfc3339(x.c_str(), x.size());
}

TEST(IsDateTimeCheck, Date) {
    EXPECT_TRUE(is_date("2021-12-12"));
    EXPECT_TRUE(is_date("2021-12-02"));
    EXPECT_TRUE(is_date("2021-07-30"));
    EXPECT_TRUE(is_date("2021-07-31"));
    EXPECT_TRUE(is_date("5277-01-31"));

    EXPECT_FALSE(is_date("short"));
    EXPECT_FALSE(is_date("long long long long"));
    EXPECT_FALSE(is_date("asdasdasda"));
    EXPECT_FALSE(is_date("aa20-12-12"));
    EXPECT_FALSE(is_date("2021-a2-12"));
    EXPECT_FALSE(is_date("2021-12-b2"));
    EXPECT_FALSE(is_date("2021:07-30"));
    EXPECT_FALSE(is_date("2021-07:30"));
    EXPECT_FALSE(is_date("2021-55-31"));
    EXPECT_FALSE(is_date("5277-01-32"));
}

TEST(IsDateTimeCheck, DateTime) {
    EXPECT_TRUE(is_rfc3339("2077-12-12T22:11:00Z"));
    EXPECT_TRUE(is_rfc3339("2055-01-01T05:34:12+19:11"));
    EXPECT_TRUE(is_rfc3339("2022-05-06T24:00:00-02:12"));
    EXPECT_TRUE(is_rfc3339("2022-05-06T24:00:00.000+02:12"));
    EXPECT_TRUE(is_rfc3339("2022-05-06T13:00:00.334-02:12"));
    EXPECT_TRUE(is_rfc3339("2022-05-06T23:59:60Z"));

    EXPECT_FALSE(is_rfc3339("2055-99-01T05:34:12"));
    EXPECT_FALSE(is_rfc3339("2055-99-01T05:34:12+19:11"));
    EXPECT_FALSE(is_rfc3339("2055-99-01T05:34:12"));

    EXPECT_FALSE(is_rfc3339("2055-12-01T11:5511Z"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T1155:11-09:55"));
    EXPECT_FALSE(is_rfc3339("2055-12-01Ta1:55:11Z"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T11:b5:11-09:55"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T11:55:c1-09:55"));

    EXPECT_FALSE(is_rfc3339("2055-12-01T25:12:11-09:55"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T30:12:11-09:55"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T23:60:11-09:55"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T23:59:60.1Z"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T24:01:00Z"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T24:00:01Z"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T24:00:0.1Z"));

    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00.23"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00.Z"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00Z."));
    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00A"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00.A"));

    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00-a0:99"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00-10:bc"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00-99"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00-99:55"));
    EXPECT_FALSE(is_rfc3339("2055-12-01T12:00:00-00:99"));
}
