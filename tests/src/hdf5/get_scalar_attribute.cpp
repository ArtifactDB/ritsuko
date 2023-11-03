#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ritsuko/hdf5/get_scalar_attribute.hpp"
#include "utils.h"

#include <numeric>
#include <string>

static void expect_error(const H5::Group& handle, const char* name, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::get_scalar_attribute(handle, name);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(GetScalarAttribute, Basic) {
    std::string path = "TEST-attribute.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("whee");
        ghandle.createAttribute("okay", H5::PredType::NATIVE_INT, H5S_SCALAR);
        hsize_t ndim = 100;
        H5::DataSpace dspace(1, &ndim);
        ghandle.createAttribute("nooo", H5::PredType::NATIVE_INT, dspace);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto ghandle = handle.openGroup("whee");
    auto attr = ritsuko::hdf5::get_scalar_attribute(ghandle, "okay");
    EXPECT_EQ(attr.getTypeClass(), H5T_INTEGER);
    expect_error(ghandle, "missing", "expected an attribute");
    expect_error(ghandle, "nooo", "expected a scalar attribute");
}
