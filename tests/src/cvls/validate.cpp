#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstdint>

#include "ritsuko/cvls/validate.hpp"
#include "ritsuko/cvls/Pointer.hpp"

#include "utils.h"
#include "../hdf5/utils.h"

TEST(CvlsValidatePointers, GeneralErrors) {
    // Creating a file.
    const std::string path = "TEST-vls-pointer.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        auto dtype = ritsuko::cvls::define_pointer_datatype<uint32_t, uint32_t>();
        size_t nlen = 10;
        std::vector<ritsuko::cvls::Pointer<uint32_t, uint32_t> > data(nlen);
        for (size_t i = 0; i < nlen; ++i) {
            data[i].offset = i * 1;
            data[i].length = i * 100;
        }
        create_vls_pointer_dataset(handle, "foo", data, dtype);

        std::vector<double> fdata(nlen);
        create_dataset(handle, "bar", fdata, H5::PredType::NATIVE_DOUBLE);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);

    // Checking that it throws if the type class is not correct.
    {
        std::string msg;
        auto xhandle = handle.openDataSet("bar");
        try {
            ritsuko::cvls::validate_pointers<std::uint32_t, std::uint32_t>(xhandle);
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("compound"));
    }

    // Checking that it throws if the pointer type is not correct.
    {
        std::string msg;
        auto xhandle = handle.openDataSet("foo");
        try {
            ritsuko::cvls::validate_pointers<std::int32_t, std::uint32_t>(xhandle);
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("incorrect type"));
    }
}

class CvlsValidatePointersTest : public ::testing::TestWithParam<bool> {};

TEST_P(CvlsValidatePointersTest, OneDim) {
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

TEST_P(CvlsValidatePointersTest, NDim) {
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
            cplist.setDeflate(6);
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
    CvlsValidatePointers,
    CvlsValidatePointersTest,
    ::testing::Values(false, true)
);

TEST(CvlsValidatePointers, OneDimError) {
    const std::string path = "TEST-vls-validate.h5";
    hsize_t nlen = 103; 
    hsize_t heap = 100;

    auto dtype = ritsuko::cvls::define_pointer_datatype<uint32_t, uint32_t>();
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace(1, &nlen);
        auto dhandle = handle.createDataSet("foobar", dtype, dspace);
    }

    std::vector<ritsuko::cvls::Pointer<uint32_t, uint32_t> > data(nlen);
    for (size_t i = 0; i < nlen; ++i) {
        data[i].offset = 0;
        data[i].length = 10;
    }

    // Injecting errors at different locations to check that we actually iterate through the entire dataset.
    for (int scenario = 0; scenario < 3; ++scenario) {
        std::size_t loc; 
        if (scenario == 0) {
            loc = 0;
        } else if (scenario == 1) {
            loc = nlen / 2;
        } else {
            loc = nlen - 1;
        }

        // Trying different failure modes.
        auto previous = data[loc];
        if (scenario == 0) {
            // End above the limit.
            data[loc].offset = heap - 1;
        } else {
            // Start above the limit.
            data[loc].offset = heap + 1;
        }

        {
            H5::H5File handle(path, H5F_ACC_RDWR);
            auto dhandle = handle.openDataSet("foobar");
            dhandle.write(data.data(), dtype);
        }

        {
            H5::H5File handle(path, H5F_ACC_RDONLY);
            auto dhandle = handle.openDataSet("foobar");
            std::string msg;
            try {
                ritsuko::cvls::validate_1d_pointers<std::uint64_t, std::uint64_t>(dhandle, nlen, heap);
            } catch (std::exception& e) {
                msg = e.what();
            }
            EXPECT_THAT(msg, ::testing::HasSubstr("out of range"));
        }

        data[loc] = previous;
    }
}

TEST(CvlsValidatePointers, NDimErrors) {
    const std::string path = "TEST-vls-validate.h5";
    std::vector<hsize_t> dims{ 78, 51 };
    hsize_t nlen = dims[0] * dims[1];
    hsize_t heap = 100;

    auto dtype = ritsuko::cvls::define_pointer_datatype<uint32_t, uint32_t>();
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace(2, dims.data());
        auto dhandle = handle.createDataSet("foobar", dtype, dspace);
    }

    std::vector<ritsuko::cvls::Pointer<uint32_t, uint32_t> > data(nlen);
    for (size_t i = 0; i < nlen; ++i) {
        data[i].offset = 0;
        data[i].length = 10;
    }

    // Injecting errors at different locations to check that we actually iterate through the entire dataset.
    for (int scenario = 0; scenario < 3; ++scenario) {
        std::size_t loc; 
        if (scenario == 0) {
            loc = 0;
        } else if (scenario == 1) {
            loc = nlen / 2;
        } else {
            loc = nlen - 1;
        }

        auto previous = data[loc];
        if (scenario == 0) {
            // Start above the limit.
            data[loc].offset = heap + 1;
        } else {
            // End above the limit.
            data[loc].offset = heap - 1;
        }

        {
            H5::H5File handle(path, H5F_ACC_RDWR);
            auto dhandle = handle.openDataSet("foobar");
            dhandle.write(data.data(), dtype);
        }

        {
            H5::H5File handle(path, H5F_ACC_RDONLY);
            auto dhandle = handle.openDataSet("foobar");
            std::string msg;
            try {
                ritsuko::cvls::validate_nd_pointers<std::uint64_t, std::uint64_t>(dhandle, dims, heap);
            } catch (std::exception& e) {
                msg = e.what();
            }
            EXPECT_THAT(msg, ::testing::HasSubstr("out of range"));
        }

        data[loc] = previous;
    }
}

TEST(CvlsValidatePointers, Scalar) {
    const std::string path = "TEST-vls-validate.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dtype = ritsuko::cvls::define_pointer_datatype<std::uint32_t, std::uint32_t>();
        ritsuko::cvls::Pointer<std::uint32_t, std::uint32_t> data;
        data.offset = 0;
        data.length = 10;

        H5::DataSpace dspace;
        auto dhandle = handle.createDataSet("stuff", dtype, dspace);
        dhandle.write(&data, dtype);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("stuff");
    ritsuko::cvls::validate_scalar_pointer<std::uint64_t, std::uint64_t>(dhandle, 100);

    // Fails if pointers are out of range.
    {
        std::string errmsg = "no_error";
        try {
            ritsuko::cvls::validate_scalar_pointer<std::uint64_t, std::uint64_t>(dhandle, 5);
        } catch (std::exception& e) {
            errmsg = e.what();
        }
        EXPECT_THAT(errmsg, ::testing::HasSubstr("out of range"));
    }

    // Fails if the type is too small.
    {
        std::string errmsg = "no_error";
        try {
            ritsuko::cvls::validate_scalar_pointer<std::uint16_t, std::uint16_t>(dhandle, 100);
        } catch (std::exception& e) {
            errmsg = e.what();
        }
        EXPECT_THAT(errmsg, ::testing::HasSubstr("incorrect type"));
    }
}

TEST(CvlsValidateHeap, Basic) {
    const std::string path = "TEST-vls-heap.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        size_t nlen = 10;
        std::vector<uint8_t> data(nlen);
        create_dataset(handle, "foo", data, H5::PredType::NATIVE_UINT8);

        std::vector<int> idata(nlen);
        create_dataset(handle, "bar", idata, H5::PredType::NATIVE_INT32);

        std::vector<double> ddata(nlen);
        create_dataset(handle, "other", ddata, H5::PredType::NATIVE_DOUBLE);

        {
            std::vector<hsize_t> dims{ 77, 192 };
            H5::DataSpace dspace(2, dims.data());
            handle.createDataSet("more", H5::PredType::NATIVE_UINT8, dspace);
        }
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foo");
    ritsuko::cvls::validate_heap(dhandle);

    {
        auto xhandle = handle.openDataSet("bar");
        std::string msg;
        try {
            ritsuko::cvls::validate_heap(xhandle);
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("8-bit unsigned integers"));
    }

    {
        auto xhandle = handle.openDataSet("other");
        std::string msg;
        try {
            ritsuko::cvls::validate_heap(xhandle);
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("expected an integer"));
    }

    {
        auto xhandle = handle.openDataSet("more");
        std::string msg;
        try {
            ritsuko::cvls::validate_heap(xhandle);
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("1-dimensional"));
    }
}

