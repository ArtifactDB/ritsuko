#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/missing_placeholder.hpp"
#include "utils.h"
#include <numeric>
#include <string>

TEST(Hdf5MissingPlaceholder, Loading) {
    const char* path = "TEST-missing-placeholder.h5";

   // Integers.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = create_dataset(handle, "foobar", std::vector<int>{1,2,3}, H5::PredType::NATIVE_INT);
        auto ahandle = dhandle.createAttribute("ouch", H5::PredType::NATIVE_INT, H5S_SCALAR);
        int val = 123;
        ahandle.write(H5::PredType::NATIVE_INT, &val);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        auto found = ritsuko::hdf5::open_and_load_optional_numeric_missing_placeholder<int32_t>(dhandle, "ouch");
        EXPECT_TRUE(found.has_value());
        EXPECT_EQ(found.value(), 123);
    }

    // Double precision.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = create_dataset(handle, "foobar", std::vector<double>{1,2,3}, H5::PredType::NATIVE_DOUBLE);
        auto ahandle = dhandle.createAttribute("ouch", H5::PredType::NATIVE_DOUBLE, H5S_SCALAR);
        double val = 1.5;
        ahandle.write(H5::PredType::NATIVE_DOUBLE, &val);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        auto found = ritsuko::hdf5::open_and_load_optional_numeric_missing_placeholder<double>(dhandle, "ouch");
        EXPECT_TRUE(found.has_value());
        EXPECT_EQ(found.value(), 1.5);
    }

    // String.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = create_dataset(handle, "foobar", std::vector<std::string>{"A", "BB", "CCC"}, false);
        H5::StrType stype(0, H5T_VARIABLE);
        auto ahandle = dhandle.createAttribute("ouch", stype, H5S_SCALAR);
        ahandle.write(stype, std::string("YAY"));
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        auto found = ritsuko::hdf5::open_and_load_optional_string_missing_placeholder(dhandle, "ouch");
        EXPECT_TRUE(found.has_value());
        EXPECT_EQ(found.value(), std::string("YAY"));
    }

    // Not even present.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", std::vector<int>{1,2,3}, H5::PredType::NATIVE_INT32);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        EXPECT_FALSE(ritsuko::hdf5::open_and_load_optional_numeric_missing_placeholder<int32_t>(dhandle, "ouch").has_value());
        EXPECT_FALSE(ritsuko::hdf5::open_and_load_optional_string_missing_placeholder(dhandle, "ouch").has_value());
    }
}

template<typename ... Args_>
static void expect_numeric_error(std::string msg, Args_&& ... args) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::check_numeric_missing_placeholder_attribute(std::forward<Args_>(args)...);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5MissingPlaceholder, CheckNumeric) {
    const char* path = "TEST-missing-placeholder.h5";

    // Different type.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = create_dataset(handle, "foobar", std::vector<int>{1,2,3}, H5::PredType::NATIVE_INT32);
        dhandle.createAttribute("ouch", H5::PredType::NATIVE_INT16, H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        auto ahandle = dhandle.openAttribute("ouch");
        expect_numeric_error("same type as", dhandle, ahandle);
        ritsuko::hdf5::check_numeric_missing_placeholder_attribute(dhandle, ahandle, true);
    }

    // Different type class.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = create_dataset(handle, "foobar", std::vector<int>{1,2,3}, H5::PredType::NATIVE_INT32);
        dhandle.createAttribute("ouch", H5::PredType::NATIVE_DOUBLE, H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        auto ahandle = dhandle.openAttribute("ouch");
        expect_numeric_error("same type as", dhandle, ahandle);
        expect_numeric_error("same type class as", dhandle, ahandle, true);
    }

    // Non-scalar dataset.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = create_dataset(handle, "foobar", std::vector<int>{1,2,3}, H5::PredType::NATIVE_INT32);
        hsize_t len = 10;
        H5::DataSpace space(1, &len);
        dhandle.createAttribute("ouch", H5::PredType::NATIVE_INT32, space);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        auto ahandle = dhandle.openAttribute("ouch");
        expect_numeric_error("scalar", dhandle, ahandle);
    }
}

template<typename ... Args_>
static void expect_string_error(std::string msg, Args_&& ... args) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::check_string_missing_placeholder_attribute(std::forward<Args_>(args)...);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5MissingPlaceholder, CheckString) {
    const char* path = "TEST-missing-placeholder.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = create_dataset(handle, "foobar", std::vector<std::string>{"1", "2", "3"}, true);
        dhandle.createAttribute("ouch", H5::StrType(0, 10), H5S_SCALAR);
        dhandle.createAttribute("pain", H5::PredType::NATIVE_INT32, H5S_SCALAR);
        dhandle.createAttribute("stuff", H5::StrType(0, H5T_VARIABLE), H5S_SCALAR);

        hsize_t len = 10;
        H5::DataSpace dspace(1, &len);
        dhandle.createAttribute("whee", H5::StrType(0, 10), dspace);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    auto ahandle = dhandle.openAttribute("ouch");
    ritsuko::hdf5::check_string_missing_placeholder_attribute(ahandle);
    expect_string_error("string datatype", dhandle.openAttribute("pain"));
    expect_string_error("NULL pointer", dhandle.openAttribute("stuff"));
    expect_string_error("scalar", dhandle.openAttribute("whee"));
}
