#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstdint>

#include "ritsuko/cvls/validate_array.hpp"
#include "ritsuko/cvls/Pointer.hpp"

#include "utils.h"
#include "../hdf5/utils.h"

class CvlsValidateArrayTest : public ::testing::TestWithParam<bool> {};

TEST_P(CvlsValidateArrayTest, OneDim) {
    const bool chunked = GetParam();
    const std::string path = "TEST-vls-validate.h5";
    size_t nlen = 1000;

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dtype = ritsuko::cvls::define_pointer_datatype<std::uint32_t, std::uint32_t>();
        std::vector<ritsuko::cvls::Pointer<std::uint32_t, std::uint32_t> > data(nlen);
        for (size_t i = 0; i < nlen; ++i) {
            data[i].offset = i * 1;
            data[i].length = i * 10;
        }
        create_vls_pointer_dataset(handle, "foo", data, dtype, /* chunk_size = */ (chunked ? 13 : 0));
    }

    // Regular validation works as expected.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foo");
    ritsuko::cvls::validate_1d_pointers<std::uint64_t, std::uint64_t>(dhandle, nlen, 20000);

    // Fails if pointers are out of range.
    {
        std::string errmsg = "no_error";
        try {
            ritsuko::cvls::validate_1d_pointers<std::uint64_t, std::uint64_t>(dhandle, nlen, 20);
        } catch (std::exception& e) {
            errmsg = e.what();
        }
        EXPECT_THAT(errmsg, ::testing::HasSubstr("out of range"));
    }

    // Fails if the type is too small.
    {
        std::string errmsg = "no_error";
        try {
            ritsuko::cvls::validate_1d_pointers<std::uint16_t, std::uint16_t>(dhandle, nlen, 20000);
        } catch (std::exception& e) {
            errmsg = e.what();
        }
        EXPECT_THAT(errmsg, ::testing::HasSubstr("incorrect type"));
    }
}

TEST_P(CvlsValidateArrayTest, NDim) {
    const bool chunked = GetParam();
    const std::string path = "TEST-vls-validate.h5";
    std::vector<hsize_t> dims{ 131, 211 };

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        size_t nlen = dims[0] * dims[1];
        std::vector<ritsuko::cvls::Pointer<uint32_t, uint32_t> > data(nlen);
        for (size_t i = 0; i < nlen; ++i) {
            data[i].offset = i * 1;
            data[i].length = i * 10;
        }

        H5::DataSpace dspace(2, dims.data());
        H5::DSetCreatPropList cplist;
        if (chunked) {
            std::vector<hsize_t> chunks{ 11, 19 };
            cplist.setChunk(2, chunks.data());
        }

        auto dtype = ritsuko::cvls::define_pointer_datatype<uint32_t, uint32_t>();
        auto dhandle = handle.createDataSet("foobar", dtype, dspace, cplist);
        dhandle.write(data.data(), dtype);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    ritsuko::cvls::validate_nd_pointers<std::uint64_t, std::uint64_t>(dhandle, dims, 1000000);

    // Fails if pointers are out of range.
    {
        std::string errmsg = "no_error";
        try {
            ritsuko::cvls::validate_nd_pointers<std::uint64_t, std::uint64_t>(dhandle, dims, 1000);
        } catch (std::exception& e) {
            errmsg = e.what();
        }
        EXPECT_THAT(errmsg, ::testing::HasSubstr("out of range"));
    }

    // Fails if the type is too small.
    {
        std::string errmsg = "no_error";
        try {
            ritsuko::cvls::validate_nd_pointers<std::uint16_t, std::uint16_t>(dhandle, dims, 20000);
        } catch (std::exception& e) {
            errmsg = e.what();
        }
        EXPECT_THAT(errmsg, ::testing::HasSubstr("incorrect type"));
    }
}

INSTANTIATE_TEST_SUITE_P(
    CvlsValidateArray,
    CvlsValidateArrayTest,
    ::testing::Values(false, true)
);
