#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/parse_version_string.hpp"

auto parse(const std::string& version, bool skip_patch = false) {
    return ritsuko::parse_version_string(version.c_str(), version.size(), skip_patch);
}

static void expect_version_error(std::string version, std::string msg, bool skip_patch = false) {
    EXPECT_ANY_THROW({
        try {
            parse(version, skip_patch);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(VersionParsing, Checks) {
    auto out = parse("1.0.0");
    EXPECT_EQ(out.major, 1);
    EXPECT_EQ(out.minor, 0);
    EXPECT_EQ(out.patch, 0);

    out = parse("123.45.6");
    EXPECT_EQ(out.major, 123);
    EXPECT_EQ(out.minor, 45);
    EXPECT_EQ(out.patch, 6);

    out = parse("0.99", true); 
    EXPECT_EQ(out.major, 0);
    EXPECT_EQ(out.minor, 99);
    EXPECT_EQ(out.patch, 0);

    out = parse("2.0", true); 
    EXPECT_EQ(out.major, 2);
    EXPECT_EQ(out.minor, 0);
    EXPECT_EQ(out.patch, 0);
}

TEST(VersionParsing, Errors) {
    expect_version_error("", "empty");
    expect_version_error("00.1.1", "leading zeros");
    expect_version_error("a.1.1", "non-digit");
    expect_version_error("1", "minor version");
    expect_version_error("1.", "minor version");
    expect_version_error("1.01.1", "leading zeros");
    expect_version_error("1.a.1", "non-digit");
    expect_version_error("1.0", "missing a patch version");
    expect_version_error("1.0.", "missing a patch version");
    expect_version_error("1.0.00", "leading zeros");
    expect_version_error("1.0.a", "non-digit");
    expect_version_error("1.0.0", "not have a patch version", true);
}

TEST(VersionParsing, Comparisons) {
    using Version = ritsuko::Version;

    EXPECT_TRUE(Version(1, 0, 0) == Version(1, 0, 0));

    EXPECT_TRUE(Version(2, 0, 0) != Version(1, 0, 0));
    EXPECT_TRUE(Version(1, 1, 0) != Version(1, 0, 0));
    EXPECT_TRUE(Version(1, 0, 1) != Version(1, 0, 0));

    EXPECT_TRUE(Version(2, 0, 0) > Version(1, 32, 0));
    EXPECT_TRUE(Version(1, 1, 0) > Version(1, 0, 34));
    EXPECT_TRUE(Version(1, 0, 1) > Version(1, 0, 0));

    EXPECT_TRUE(Version(2, 0, 0) >= Version(1, 99, 0));
    EXPECT_TRUE(Version(1, 1, 0) >= Version(1, 0, 99));
    EXPECT_TRUE(Version(1, 0, 1) >= Version(1, 0, 0));
    EXPECT_TRUE(Version(1, 0, 0) >= Version(1, 0, 0));

    EXPECT_TRUE(Version(1, 38, 0) < Version(2, 0, 0));
    EXPECT_TRUE(Version(1, 1, 72) < Version(1, 3, 0));
    EXPECT_TRUE(Version(1, 0, 2) < Version(1, 0, 5));

    EXPECT_TRUE(Version(1, 3, 0) <= Version(2, 0, 0));
    EXPECT_TRUE(Version(1, 0, 2) <= Version(1, 3, 0));
    EXPECT_TRUE(Version(1, 0, 0) <= Version(1, 0, 5));
    EXPECT_TRUE(Version(1, 2, 0) <= Version(1, 2, 0));
}
