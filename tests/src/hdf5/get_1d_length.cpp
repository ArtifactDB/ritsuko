#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/get_1d_length.hpp"

static void expect_error(const H5::DataSpace& space, bool allow_scalar, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::get_1d_length(space, allow_scalar);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5Get1dLength, Basic) {
    hsize_t len = 100;
    { 
        H5::H5File handle("TEST-1d-length.h5", H5F_ACC_TRUNC);
        H5::DataSpace space(1, &len);
        handle.createDataSet("foo", H5::PredType::NATIVE_INT, space);
    }
    {
        H5::H5File handle("TEST-1d-length.h5", H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo");
        EXPECT_EQ(len, ritsuko::hdf5::get_1d_length(dhandle.getSpace(), false));
    }

    { 
        H5::H5File handle("TEST-1d-length.h5", H5F_ACC_TRUNC);
        hsize_t len[2] { 100, 200 };
        H5::DataSpace space(2, len);
        handle.createDataSet("foo", H5::PredType::NATIVE_INT, space);
    }
    {
        H5::H5File handle("TEST-1d-length.h5", H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo");
        expect_error(dhandle.getSpace(), false, "1-dimensional");
    }
}

TEST(Hdf5Get1dLength, Scalar) {
    { 
        H5::H5File handle("TEST-1d-length.h5", H5F_ACC_TRUNC);
        handle.createDataSet("foo", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }

    {
        H5::H5File handle("TEST-1d-length.h5", H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo");
        EXPECT_EQ(ritsuko::hdf5::get_1d_length(dhandle.getSpace(), true), 0);
        expect_error(dhandle.getSpace(), false, "1-dimensional");
    }
}
