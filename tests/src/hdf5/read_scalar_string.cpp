#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ritsuko/hdf5/read_scalar_string.hpp"

TEST(Hdf5ReadScalarString, AttributeFixed) {
    const char* path = "TEST-1d-attr.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("whee");
        H5::StrType stype(0, 10);
        auto ahandle = ghandle.createAttribute("foo1", stype, H5S_SCALAR);
        ahandle.write(stype, std::string("YAY"));
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto ghandle = handle.openGroup("whee");
    auto ahandle = ghandle.openAttribute("foo1");
    EXPECT_EQ(ritsuko::hdf5::read_scalar_string(ahandle), "YAY");
}

TEST(Hdf5ReadScalarString, AttributeVariable) {
    const char* path = "TEST-1d-attr.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("whee");
        H5::StrType stype(0, H5T_VARIABLE);
        auto ahandle = ghandle.createAttribute("foo1", stype, H5S_SCALAR);
    }

    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto ghandle = handle.openGroup("whee");
        auto ahandle = ghandle.openAttribute("foo1");
        std::string msg;
        try {
            ritsuko::hdf5::read_scalar_string(ahandle);
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("NULL pointer"));
    }

    {
        H5::H5File handle(path, H5F_ACC_RDWR);
        auto ghandle = handle.openGroup("whee");
        auto ahandle = ghandle.openAttribute("foo1");
        auto stype = ahandle.getDataType();
        ahandle.write(stype, std::string("YAY"));
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto ghandle = handle.openGroup("whee");
    auto ahandle = ghandle.openAttribute("foo1");
    EXPECT_EQ(ritsuko::hdf5::read_scalar_string(ahandle), "YAY");
}

TEST(Hdf5ReadScalarString, DataSetFixed) {
    const char* path = "TEST-1d-dset.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::StrType stype(0, 10);
        auto dhandle = handle.createDataSet("foo1", stype, H5S_SCALAR);
        dhandle.write(std::string("YAY"), stype);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foo1");
    EXPECT_EQ(ritsuko::hdf5::read_scalar_string(dhandle), "YAY");
}

TEST(Hdf5ReadScalarString, DataSetVariable) {
    const char* path = "TEST-1d-dset.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::StrType stype(0, H5T_VARIABLE);
        auto dhandle = handle.createDataSet("foo1", stype, H5S_SCALAR);
    }

    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo1");
        std::string msg;
        try {
            ritsuko::hdf5::read_scalar_string(dhandle);
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("NULL pointer"));
    }

    {
        H5::H5File handle(path, H5F_ACC_RDWR);
        auto dhandle = handle.openDataSet("foo1");
        dhandle.write(std::string("YAY"), dhandle.getDataType());
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foo1");
    EXPECT_EQ(ritsuko::hdf5::read_scalar_string(dhandle), "YAY");
}
