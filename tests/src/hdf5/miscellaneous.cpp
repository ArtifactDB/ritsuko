#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/miscellaneous.hpp"

TEST(Hdf5Miscellaneous, OpenScalarAttribute) {
    const char* path = "TEST-scalar-attr.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("INT8", H5::PredType::NATIVE_INT8, H5S_SCALAR);
        H5::StrType stype(0, H5T_VARIABLE);
        dhandle.createAttribute("Whee1", stype, H5S_SCALAR);
        hsize_t len = 10;
        H5::DataSpace dspace(1, &len);
        dhandle.createAttribute("Whee2", stype, dspace);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("INT8");
    ritsuko::hdf5::open_scalar_attribute(dhandle, "Whee1");

    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::open_scalar_attribute(dhandle, "Whee2");
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("scalar"));
            throw;
        }
    });
}

TEST(Hdf5Miscellaneous, OpenAndLoadScalarString) {
    const char* path = "TEST-1d-attr.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("whee");
        H5::StrType stype(0, 10);
        auto ahandle = ghandle.createAttribute("foo1", stype, H5S_SCALAR);
        ahandle.write(stype, std::string("YAY"));
        ghandle.createAttribute("foo2", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openGroup("whee");
    EXPECT_EQ(ritsuko::hdf5::open_and_load_scalar_string_attribute(dhandle, "foo1"), "YAY");

    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::open_and_load_scalar_string_attribute(dhandle, "foo2");
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("string"));
            throw;
        }
    });
}
