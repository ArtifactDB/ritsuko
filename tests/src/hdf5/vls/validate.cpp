#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstdint>

#include "ritsuko/hdf5/vls/open.hpp"
#include "ritsuko/hdf5/vls/validate.hpp"
#include "ritsuko/hdf5/vls/Pointer.hpp"

#include "utils.h"
#include "../utils.h"

TEST(VlsValidate, OneDim) {
    const std::string path = "TEST-vls-validate.h5";
    size_t nlen = 1000;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > data(nlen);
        for (size_t i = 0; i < nlen; ++i) {
            data[i].offset = i * 1;
            data[i].length = i * 10;
        }
        create_vls_pointer_dataset(handle, "foo", data, dtype, /* chunk_size = */ 13);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
    std::vector<size_t> buffer_sizes{ 11, 29, 53, 101 };
    for (auto buffer_size : buffer_sizes) {
        ritsuko::hdf5::vls::validate_1d_array<uint64_t, uint64_t>(dhandle, nlen, 20000, buffer_size);
    }

    std::string errmsg = "no_error";
    try {
        ritsuko::hdf5::vls::validate_1d_array<uint64_t, uint64_t>(dhandle, nlen, 20, 10);
    } catch (std::exception& e) {
        errmsg = e.what();
    }
    EXPECT_THAT(errmsg, ::testing::HasSubstr("out of range"));
}

TEST(VlsValidate, NDim) {
    const std::string path = "TEST-vls-validate.h5";
    std::vector<hsize_t> dims{ 131, 211 };

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        size_t nlen = dims[0] * dims[1];
        std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > data(nlen);
        for (size_t i = 0; i < nlen; ++i) {
            data[i].offset = i * 1;
            data[i].length = i * 10;
        }

        H5::DataSpace dspace(2, dims.data());
        H5::DSetCreatPropList cplist;
        std::vector<hsize_t> chunks{ 11, 19 };
        cplist.setChunk(2, chunks.data());

        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        auto dhandle = handle.createDataSet("foobar", dtype, dspace, cplist);
        dhandle.write(data.data(), dtype);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = ritsuko::hdf5::vls::open_pointers(handle, "foobar", 64, 64);
    std::vector<size_t> buffer_sizes{ 1000, 2000, 5000 };
    for (auto buffer_size : buffer_sizes) {
        ritsuko::hdf5::vls::validate_nd_array<uint64_t, uint64_t>(dhandle, dims, 1000000, buffer_size);
    }

    std::string errmsg = "no_error";
    try {
        ritsuko::hdf5::vls::validate_nd_array<uint64_t, uint64_t>(dhandle, dims, 1000, 10);
    } catch (std::exception& e) {
        errmsg = e.what();
    }
    EXPECT_THAT(errmsg, ::testing::HasSubstr("out of range"));
}
