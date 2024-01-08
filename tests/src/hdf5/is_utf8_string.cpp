#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/is_utf8_string.hpp"

TEST(Hdf5IsUtf8String, Safe) {
    const char* path = "TEST-utf8-string.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        {
            H5::StrType stype(0, H5T_VARIABLE);
            stype.setCset(H5T_CSET_UTF8);
            handle.createDataSet("stuff", stype, H5S_SCALAR);
            handle.createDataSet("failed", H5::PredType::NATIVE_INT, H5S_SCALAR);
        }

        {
            auto ghandle = handle.createGroup("whee");
            H5::StrType stype(0, 20);
            stype.setCset(H5T_CSET_ASCII);
            ghandle.createAttribute("foo", stype, H5S_SCALAR);
            ghandle.createAttribute("bar", H5::PredType::NATIVE_FLOAT, H5S_SCALAR);
        }
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    EXPECT_TRUE(ritsuko::hdf5::is_utf8_string(handle.openDataSet("stuff")));
    EXPECT_FALSE(ritsuko::hdf5::is_utf8_string(handle.openDataSet("failed")));

    auto ghandle = handle.openGroup("whee");
    EXPECT_TRUE(ritsuko::hdf5::is_utf8_string(ghandle.openAttribute("foo")));
    EXPECT_FALSE(ritsuko::hdf5::is_utf8_string(ghandle.openAttribute("bar")));
}
