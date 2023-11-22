#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ritsuko/hdf5/get_name.hpp"
#include "utils.h"

#include <numeric>
#include <string>

TEST(Hdf5GetName, Basic) {
    const char* path = "TEST-dataset.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        handle.createDataSet("foobar", H5::PredType::NATIVE_INT, H5S_SCALAR);
        auto ghandle = handle.createGroup("whee");
        auto xhandle = ghandle.createGroup("stuff");
        xhandle.createAttribute("blahblah", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_EQ("/foobar", ritsuko::hdf5::get_name(handle.openDataSet("foobar")));
        EXPECT_EQ("/whee", ritsuko::hdf5::get_name(handle.openGroup("whee")));

        auto xhandle = handle.openGroup("whee/stuff");
        EXPECT_EQ("/whee/stuff", ritsuko::hdf5::get_name(xhandle));

        auto ahandle = xhandle.openAttribute("blahblah");
        EXPECT_EQ("blahblah", ritsuko::hdf5::get_name(ahandle));
    }
}
