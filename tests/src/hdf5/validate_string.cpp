#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/validate_string.hpp"
#include "utils.h"
#include <numeric>
#include <string>

TEST(ValidateString, FixedNdimensional) {
    const char* path = "TEST-validate-string.h5";

    std::vector<hsize_t> dims{ 77, 192 };
    std::vector<hsize_t> chunks{ 12, 15 };
    size_t strlen = 5;
    std::vector<const char*> ptrs(strlen * dims[0] * dims[1]);

    H5::DataSpace dspace(2, dims.data());
    H5::DSetCreatPropList cplist;
    cplist.setChunk(2, chunks.data());
    H5::StrType stype(0, strlen);

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("foobar", stype, dspace, cplist);
        dhandle.write(ptrs.data(), stype);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        ritsuko::hdf5::validate_nd_string_dataset(dhandle, 100);
    }
}

TEST(ValidateString, VariableNdimensional) {
    const char* path = "TEST-validate-string.h5";

    std::vector<hsize_t> dims{ 131, 211 };
    std::vector<hsize_t> chunks{ 10, 20 };
    const char * placeholder = "foobar";
    std::vector<const char*> ptrs(dims[0] * dims[1], placeholder);

    H5::DataSpace dspace(2, dims.data());
    H5::DSetCreatPropList cplist;
    cplist.setChunk(2, chunks.data());
    H5::StrType stype(0, H5T_VARIABLE);

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("foobar", stype, dspace, cplist);
        dhandle.write(ptrs.data(), stype);
    }

    std::vector<size_t> buffer_sizes { 100, 1000, 10000, 100000 };
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        for (auto buf : buffer_sizes) {
            ritsuko::hdf5::validate_nd_string_dataset(dhandle, buf);
        }
    }

    // Injecting a NULL at every corner and checking that the validator can find it.
    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            size_t offset = i + j * dims[0];
            ptrs[offset] = NULL;

            {
                H5::H5File handle(path, H5F_ACC_RDWR);
                auto dhandle = handle.openDataSet("foobar");
                dhandle.write(ptrs.data(), stype);
            }

            H5::H5File handle(path, H5F_ACC_RDONLY);
            auto dhandle = handle.openDataSet("foobar");
            for (auto buf : buffer_sizes) {
                EXPECT_ANY_THROW({
                    try {
                        ritsuko::hdf5::validate_nd_string_dataset(dhandle, buf);
                    } catch (std::exception& e) {
                        EXPECT_THAT(e.what(), ::testing::HasSubstr("NULL pointer"));
                        throw;
                    }
                });
            }

            ptrs[offset] = placeholder;
        }
    }
}
