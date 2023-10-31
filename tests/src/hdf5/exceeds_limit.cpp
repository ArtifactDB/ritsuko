#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/exceeds_limit.hpp"

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
    handle.createDataSet("FLOAT32", H5::PredType::NATIVE_FLOAT, dspace);
    handle.createDataSet("FLOAT64", H5::PredType::NATIVE_DOUBLE, dspace);
}

TEST(Hdf5ExceedsIntegerLimit, Safe) {
    const char* path = "TEST-forbid.h5";
    spawn_file(path);

    H5::H5File handle(path, H5F_ACC_RDONLY);
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT8"), 8, true)); 
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT8"), 16, true)); 
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT8"), 8, false));
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT8"), 16, true));
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT8"), 16, false));

    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT16"), 16, true));
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT16"), 32, true));
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT16"), 16, false));
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT16"), 32, true));
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT16"), 32, false));

    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT32"), 32, true));
    EXPECT_FALSE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT32"), 32, false));
}

TEST(Hdf5ExceedsIntegerLimit, Failed) {
    const char* path = "TEST-forbid.h5";
    spawn_file(path);

    H5::H5File handle(path, H5F_ACC_RDONLY);
    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT8"), 8, true));
    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT8"), 8, false));

    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT16"), 8, true));
    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT16"), 8, false));
    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT16"), 16, false));
    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT16"), 16, true));

    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("INT32"), 32, false));
    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("UINT32"), 32, true));

    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("FLOAT32"), 32, false));
    EXPECT_TRUE(ritsuko::hdf5::exceeds_integer_limit(handle.openDataSet("FLOAT32"), 64, true));
}

TEST(Hdf5ExceedsFloatLimit, Safe) {
    const char* path = "TEST-forbid.h5";
    spawn_file(path);

    H5::H5File handle(path, H5F_ACC_RDONLY);
    EXPECT_FALSE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("INT8"), 32)); 
    EXPECT_FALSE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("UINT8"), 32)); 
    EXPECT_FALSE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("INT16"), 32)); 
    EXPECT_FALSE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("UINT16"), 32)); 

    EXPECT_FALSE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("INT32"), 64)); 
    EXPECT_FALSE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("UINT32"), 64)); 

    EXPECT_FALSE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("FLOAT32"), 32)); 
    EXPECT_FALSE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("FLOAT64"), 64)); 
}

TEST(Hdf5ExceedsFloatLimit, Failed) {
    const char* path = "TEST-forbid.h5";
    spawn_file(path);

    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_TRUE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("INT32"), 32)); 
        EXPECT_TRUE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("UINT32"), 32)); 
        EXPECT_TRUE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("INT64"), 64)); 
        EXPECT_TRUE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("FLOAT64"), 32)); 
    }

    {
        H5::H5File handle(path, H5F_ACC_RDWR);
        handle.createDataSet("STR", H5::StrType(0, H5T_VARIABLE), H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_TRUE(ritsuko::hdf5::exceeds_float_limit(handle.openDataSet("STR"), 32)); 
    }
}
