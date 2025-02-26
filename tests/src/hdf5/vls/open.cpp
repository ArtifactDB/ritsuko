#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstdint>

#include "ritsuko/hdf5/vls/open.hpp"

#include "utils.h"
#include "../utils.h"

TEST(VlsOpen, Pointers) {
    // Creating a file.
    const std::string path = "TEST-vls-pointer.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        size_t nlen = 10;
        std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > data(nlen);
        for (size_t i = 0; i < nlen; ++i) {
            data[i].start = i * 1;
            data[i].size = i * 100;
        }
        create_vls_pointer_dataset(handle, "foo", data, dtype);

        std::vector<int> idata(nlen);
        create_dataset(handle, "bar", idata, H5::PredType::NATIVE_UINT8);
    }

    // Checking that it opens correctly.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 32, 32);
    EXPECT_TRUE(dhandle.getTypeClass() == H5T_COMPOUND);

    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::vls::open_pointers(handle, "foo", 8, 8);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("exceed"));
            throw;
        }
    });

    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::vls::open_pointers(handle, "bar", 8, 8);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("compound"));
            throw;
        }
    });
}

TEST(VlsOpen, Concatenated) {
    // Creating a file.
    const std::string path = "TEST-vls-concatenated.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        size_t nlen = 10;
        std::vector<uint8_t> data(nlen);
        create_dataset(handle, "foo", data, H5::PredType::NATIVE_UINT8);

        std::vector<int> idata(nlen);
        create_dataset(handle, "bar", idata, H5::PredType::NATIVE_INT32);

        std::vector<double> ddata(nlen);
        create_dataset(handle, "other", ddata, H5::PredType::NATIVE_DOUBLE);
    }

    // Checking that it opens correctly.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = ritsuko::hdf5::vls::open_concatenated(handle, "foo");
    EXPECT_TRUE(dhandle.getTypeClass() == H5T_INTEGER);

    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::vls::open_concatenated(handle, "bar");
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("8-bit unsigned integers"));
            throw;
        }
    });

    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::vls::open_concatenated(handle, "other");
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("expected an integer"));
            throw;
        }
    });
}
