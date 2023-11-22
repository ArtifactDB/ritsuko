#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/load_scalar_dataset.hpp"

TEST(Hdf5LoadScalarDataset, String) {
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
    }
 }
