#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/load_scalar_string_attribute.hpp"

TEST(Hdf5LoadScalarStringAttribute, OkayDataset) {
    const char* path = "TEST-scalar-attr.h5";

    const char* attr_name = "my_attr";
    std::string attr_val = "fooble";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("INT8", H5::PredType::NATIVE_INT8, H5S_SCALAR);
        H5::StrType stype(0, H5T_VARIABLE);
        auto ahandle = dhandle.createAttribute(attr_name, stype, H5S_SCALAR);
        ahandle.write(stype, attr_val);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("INT8");
    EXPECT_EQ(ritsuko::hdf5::load_scalar_string_attribute(dhandle, attr_name, "INT8"), attr_val);
}

TEST(Hdf5LoadScalarStringAttribute, OkayGroup) {
    const char* path = "TEST-scalar-attr.h5";

    const char* attr_name = "my_attr2";
    std::string attr_val = "supercagilfrag";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("UINT8");
        H5::StrType stype(0, H5T_VARIABLE);
        auto ahandle = ghandle.createAttribute(attr_name, stype, H5S_SCALAR);
        ahandle.write(stype, attr_val);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto ghandle = handle.openGroup("UINT8");
    EXPECT_EQ(ritsuko::hdf5::load_scalar_string_attribute(ghandle, attr_name, "UINT8"), attr_val);
}

static void expect_error(const H5::Group& handle, const char* field, const char* name, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::load_scalar_string_attribute(handle, field, name);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5LoadScalarStringAttribute, Failures) {
    const char* path = "TEST-scalar-attr.h5";

    const char* attr_name = "nagisa";
    std::string attr_val = "tomoya";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("UINT8");
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        expect_error(handle.openGroup("UINT8"), attr_name, "UINT8", "'nagisa'");
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("UINT8");
        ghandle.createAttribute(attr_name, H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        expect_error(handle.openGroup("UINT8"), attr_name, "UINT8", "scalar string");
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("UINT8");
        H5::StrType stype(0, H5T_VARIABLE);
        hsize_t len = 10;
        H5::DataSpace dspace(1, &len);
        ghandle.createAttribute(attr_name, H5::PredType::NATIVE_INT, dspace);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        expect_error(handle.openGroup("UINT8"), attr_name, "UINT8", "scalar string");
    }
}
