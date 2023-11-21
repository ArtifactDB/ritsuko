#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/as_numeric_datatype.hpp"

TEST(ChooseDatatype, UnsignedIntegers) {
    {
        auto x = ritsuko::hdf5::as_numeric_datatype<uint8_t>();
        EXPECT_EQ(x.getClass(), H5T_INTEGER);
        H5::IntType y(x);
        EXPECT_EQ(y.getSign(), H5T_SGN_NONE);
        EXPECT_EQ(y.getPrecision(), 8);
    }

    {
        auto x = ritsuko::hdf5::as_numeric_datatype<uint16_t>();
        EXPECT_EQ(x.getClass(), H5T_INTEGER);
        H5::IntType y(x);
        EXPECT_EQ(y.getSign(), H5T_SGN_NONE);
        EXPECT_EQ(y.getPrecision(), 16);
    }

    {
        auto x = ritsuko::hdf5::as_numeric_datatype<uint32_t>();
        EXPECT_EQ(x.getClass(), H5T_INTEGER);
        H5::IntType y(x);
        EXPECT_EQ(y.getSign(), H5T_SGN_NONE);
        EXPECT_EQ(y.getPrecision(), 32);
    }

    {
        auto x = ritsuko::hdf5::as_numeric_datatype<uint64_t>();
        EXPECT_EQ(x.getClass(), H5T_INTEGER);
        H5::IntType y(x);
        EXPECT_EQ(y.getSign(), H5T_SGN_NONE);
        EXPECT_EQ(y.getPrecision(), 64);
    }
}

TEST(ChooseDatatype, SignedIntegers) {
    {
        auto x = ritsuko::hdf5::as_numeric_datatype<int8_t>();
        EXPECT_EQ(x.getClass(), H5T_INTEGER);
        H5::IntType y(x);
        EXPECT_EQ(y.getSign(), H5T_SGN_2);
        EXPECT_EQ(y.getPrecision(), 8);
    }

    {
        auto x = ritsuko::hdf5::as_numeric_datatype<int16_t>();
        EXPECT_EQ(x.getClass(), H5T_INTEGER);
        H5::IntType y(x);
        EXPECT_EQ(y.getSign(), H5T_SGN_2);
        EXPECT_EQ(y.getPrecision(), 16);
    }

    {
        auto x = ritsuko::hdf5::as_numeric_datatype<int32_t>();
        EXPECT_EQ(x.getClass(), H5T_INTEGER);
        H5::IntType y(x);
        EXPECT_EQ(y.getSign(), H5T_SGN_2);
        EXPECT_EQ(y.getPrecision(), 32);
    }

    {
        auto x = ritsuko::hdf5::as_numeric_datatype<int64_t>();
        EXPECT_EQ(x.getClass(), H5T_INTEGER);
        H5::IntType y(x);
        EXPECT_EQ(y.getSign(), H5T_SGN_2);
        EXPECT_EQ(y.getPrecision(), 64);
    }
}

TEST(ChooseDatatype, Floats) {
    {
        auto x = ritsuko::hdf5::as_numeric_datatype<float>();
        EXPECT_EQ(x.getClass(), H5T_FLOAT);
        H5::FloatType y(x);
        EXPECT_EQ(y.getPrecision(), 32);
    }

    {
        auto x = ritsuko::hdf5::as_numeric_datatype<double>();
        EXPECT_EQ(x.getClass(), H5T_FLOAT);
        H5::FloatType y(x);
        EXPECT_EQ(y.getPrecision(), 64);
    }
}
