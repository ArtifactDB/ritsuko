#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ritsuko/hdf5/get_group.hpp"
#include "utils.h"

static void expect_error(const H5::Group& handle, const char* name, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::get_group(handle, name);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5GetGroup, Basic) {
    const char* path = "TEST-group.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        handle.createGroup("foobar");
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        ritsuko::hdf5::get_group(handle, "foobar");
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        handle.createDataSet("foobar", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        expect_error(handle, "foobar", "expected a group");
        expect_error(handle, "blahblah", "expected a group");
    }
}
