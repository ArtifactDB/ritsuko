#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/load_dataset.hpp"
#include "ritsuko/hdf5/validate_string.hpp"
#include "utils.h"

TEST(Hdf5LoadDataset, ScalarString) {
    const char* path = "TEST-scalar-attr.h5";

    // Variable.
    {
        std::string value("barry");
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            H5::StrType stype(0, H5T_VARIABLE);
            auto dhandle = handle.createDataSet("foo", stype, H5S_SCALAR);
            const char* bptr = value.c_str();
            dhandle.write(&bptr, stype);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo");
        EXPECT_EQ(ritsuko::hdf5::load_scalar_string_dataset(dhandle), value);
        ritsuko::hdf5::validate_scalar_string_dataset(dhandle);
    }

    // Fixed.
    {
        std::string value("harriet");
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            H5::StrType stype(0, 20);
            auto dhandle = handle.createDataSet("UINT8", stype, H5S_SCALAR);
            std::vector<char> buffer(stype.getSize());
            std::copy(value.begin(), value.end(), buffer.begin());
            dhandle.write(buffer.data(), stype);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("UINT8");
        EXPECT_EQ(ritsuko::hdf5::load_scalar_string_dataset(dhandle), value);
        ritsuko::hdf5::validate_scalar_string_dataset(dhandle);
    }

    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            handle.createDataSet("stuff", H5::StrType(0, H5T_VARIABLE), H5S_SCALAR);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("stuff");
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::load_scalar_string_dataset(dhandle);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("NULL pointer"));
                throw;
            }
        });

        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::validate_scalar_string_dataset(dhandle);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("NULL pointer"));
                throw;
            }
        });
    }
}

TEST(Hdf5LoadDataset, String1d) {
    const char* path = "TEST-string1d.h5";
    std::vector<std::string> values { "kaori", "fuyuki", "shizuka", "meguru", "hiori" };

    // Variable.
    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            create_dataset(handle, "blah", values, true);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("blah");
        EXPECT_EQ(ritsuko::hdf5::load_1d_string_dataset(dhandle, 10), values);
        ritsuko::hdf5::validate_1d_string_dataset(dhandle, 10);
    }

    // Fixed.
    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            create_dataset(handle, "blah", values, false);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("blah");
        EXPECT_EQ(ritsuko::hdf5::load_1d_string_dataset(dhandle, 10), values);
        ritsuko::hdf5::validate_1d_string_dataset(dhandle, 10);
    }

    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            hsize_t len = 99;
            H5::DataSpace dspace(1, &len);
            handle.createDataSet("stuff", H5::StrType(0, H5T_VARIABLE), dspace);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("stuff");
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::load_1d_string_dataset(dhandle, 10);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("NULL pointer"));
                throw;
            }
        });

        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::validate_1d_string_dataset(dhandle, 10);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("NULL pointer"));
                throw;
            }
        });
    }
} 
