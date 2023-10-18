#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/forbid_large_integers.hpp"

static void expect_error(const H5::DataSet& dhandle, size_t limit, const char* name, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::forbid_large_integers(dhandle, limit, name);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

static void spawn_file(const char* path) {
    H5::H5File handle(path, H5F_ACC_TRUNC);
    hsize_t len = 10;
    H5::DataSpace dspace(1, &len);
    handle.createDataSet("INT8", H5::PredType::NATIVE_INT8, dspace);
    handle.createDataSet("UINT8", H5::PredType::NATIVE_UINT8, dspace);
    handle.createDataSet("INT16", H5::PredType::NATIVE_INT16, dspace);
    handle.createDataSet("UINT16", H5::PredType::NATIVE_UINT16, dspace);
    handle.createDataSet("INT32", H5::PredType::NATIVE_INT32, dspace);
    handle.createDataSet("UINT32", H5::PredType::NATIVE_UINT32, dspace);
    handle.createDataSet("INT64", H5::PredType::NATIVE_INT64, dspace);
}

TEST(Hdf5ForbidLargeIntegers, Safe) {
    const char* path = "TEST-forbid.h5";
    spawn_file(path);

    H5::H5File handle(path, H5F_ACC_RDONLY);
    ritsuko::hdf5::forbid_large_integers(handle.openDataSet("INT8"), 8, "INT8");
    ritsuko::hdf5::forbid_large_integers(handle.openDataSet("UINT8"), 16, "UINT8");
    ritsuko::hdf5::forbid_large_integers(handle.openDataSet("INT16"), 16, "INT16");
    ritsuko::hdf5::forbid_large_integers(handle.openDataSet("UINT16"), 32, "UINT16");
    ritsuko::hdf5::forbid_large_integers(handle.openDataSet("INT32"), 32, "INT32");
}

TEST(Hdf5ForbidLargeIntegers, Failed) {
    const char* path = "TEST-forbid.h5";
    spawn_file(path);

    H5::H5File handle(path, H5F_ACC_RDONLY);
    expect_error(handle.openDataSet("UINT8"), 8, "UINT8", "8-bit");
    expect_error(handle.openDataSet("INT16"), 8, "INT16", "8-bit");
    expect_error(handle.openDataSet("UINT32"), 32, "UINT32", "32-bit");
    expect_error(handle.openDataSet("INT64"), 32, "INT64", "32-bit");
}
