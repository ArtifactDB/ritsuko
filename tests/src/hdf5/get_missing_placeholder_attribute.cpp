#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/get_missing_placeholder_attribute.hpp"
#include "utils.h"
#include <numeric>
#include <string>

TEST(Hdf5GetMissingPlaceholderAttribute, Basic) {
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
        auto found = ritsuko::hdf5::load_numeric_missing_placeholder<int32_t>(dhandle, "ouch");
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, 123);
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
        auto found = ritsuko::hdf5::load_numeric_missing_placeholder<double>(dhandle, "ouch");
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, 1.5);
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
        auto found = ritsuko::hdf5::load_string_missing_placeholder(dhandle, "ouch");
        EXPECT_TRUE(found.first);
        EXPECT_EQ(found.second, std::string("YAY"));
    }
}

static void expect_error(const H5::DataSet& handle, const char* field, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::get_missing_placeholder_attribute(handle, field);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5GetMissingPlaceholderAttribute, Failed) {
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
        expect_error(dhandle, "ouch", "same type as");
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
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::get_missing_placeholder_attribute(dhandle, "ouch", true);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("same type class"));
                throw;
            }
        });
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
        expect_error(dhandle, "ouch", "scalar");
    }

    // Not even present.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", std::vector<int>{1,2,3}, H5::PredType::NATIVE_INT32);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        EXPECT_FALSE(ritsuko::hdf5::load_numeric_missing_placeholder<int32_t>(dhandle, "ouch").first);
        EXPECT_FALSE(ritsuko::hdf5::load_string_missing_placeholder(dhandle, "ouch").first);
    }
}
