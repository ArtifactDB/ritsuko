#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ritsuko/hdf5/get_dataset.hpp"
#include "utils.h"

#include <numeric>
#include <string>

static void expect_error(const H5::Group& handle, const char* name, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::get_dataset(handle, name);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5GetDataset, Basic) {
    const char* path = "TEST-dataset.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", std::vector<int>{1,2,3}, H5::PredType::NATIVE_INT);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = ritsuko::hdf5::get_dataset(handle, "foobar");
        EXPECT_EQ(dhandle.getTypeClass(), H5T_INTEGER);
        EXPECT_EQ(dhandle.getSpace().getSimpleExtentNdims(), 1);
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        handle.createGroup("blahblah");
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        expect_error(handle, "foobar", "expected a dataset");
        expect_error(handle, "blahblah", "expected a dataset");
    }
}

TEST(Hdf5GetDataset, Scalar) {
    const char* path = "TEST-dataset.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        handle.createDataSet("foobar", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = ritsuko::hdf5::get_scalar_dataset(handle, "foobar");
        EXPECT_EQ(dhandle.getTypeClass(), H5T_INTEGER);
        EXPECT_EQ(dhandle.getSpace().getSimpleExtentNdims(), 0);
    }
}
