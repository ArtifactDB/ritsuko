#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/get_dimensions.hpp"

static void expect_error(const H5::DataSpace& space, bool allow_scalar, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::get_dimensions(space, allow_scalar);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5GetDimensions, Basic1D) {
    std::vector<hsize_t> len { 100 };

    { 
        H5::H5File handle("TEST-dimensions.h5", H5F_ACC_TRUNC);
        H5::DataSpace space(1, len.data());
        handle.createDataSet("foo", H5::PredType::NATIVE_INT, space);
    }
    {
        H5::H5File handle("TEST-dimensions.h5", H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo");
        EXPECT_EQ(len, ritsuko::hdf5::get_dimensions(dhandle, false));
    }

    { 
        H5::H5File handle("TEST-dimensions.h5", H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("foo");
        H5::DataSpace space(1, len.data());
        ghandle.createAttribute("bar", H5::PredType::NATIVE_INT, space);
    }
    {
        H5::H5File handle("TEST-dimensions.h5", H5F_ACC_RDONLY);
        auto dhandle = handle.openGroup("foo");
        auto ahandle = dhandle.openAttribute("bar");
        EXPECT_EQ(len, ritsuko::hdf5::get_dimensions(ahandle, false));
    }
}

TEST(Hdf5GetDimensions, Basic2D) {
    std::vector<hsize_t> len { 100, 200 };

    { 
        H5::H5File handle("TEST-dimensions.h5", H5F_ACC_TRUNC);
        H5::DataSpace space(2, len.data());
        handle.createDataSet("foo", H5::PredType::NATIVE_INT, space);
    }
    {
        H5::H5File handle("TEST-dimensions.h5", H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo");
        EXPECT_EQ(len, ritsuko::hdf5::get_dimensions(dhandle, false));
    }
}

TEST(Hdf5GetDimensions, Scalar) {
    { 
        H5::H5File handle("TEST-dimensions.h5", H5F_ACC_TRUNC);
        handle.createDataSet("foo", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }

    {
        H5::H5File handle("TEST-dimensions.h5", H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo");
        EXPECT_TRUE(ritsuko::hdf5::get_dimensions(dhandle.getSpace(), true).empty());
        expect_error(dhandle.getSpace(), false, "N-dimensional");
    }
}
