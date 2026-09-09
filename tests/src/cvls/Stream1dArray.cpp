#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstdint>
#include <random>

#include "ritsuko/cvls/Stream1dArray.hpp"
#include "ritsuko/cvls/Pointer.hpp"

#include "utils.h"
#include "../hdf5/utils.h"

static size_t fill_pointers(const std::vector<std::string>& example, std::vector<ritsuko::cvls::Pointer<uint32_t, uint32_t> >& pointers, size_t extra = 0) {
    size_t nlen = example.size();
    size_t count = 0;
    for (size_t i = 0; i < nlen; ++i) {
        pointers[i].offset = count;
        auto ex_size = example[i].size() + extra; // possibly adding some extra stuff null terminators to the end.
        pointers[i].length = ex_size;
        count += ex_size;
    }
    return count;
}

static std::vector<unsigned char> create_heap(const std::vector<std::string>& example, size_t count, size_t extra = 0) {
    std::vector<unsigned char> heap;
    heap.reserve(count);
    for (const auto& ex : example) {
        auto ptr = reinterpret_cast<const unsigned char*>(ex.c_str());
        heap.insert(heap.end(), ptr, ptr + ex.size());
        if (extra) {
            heap.insert(heap.end(), extra, '\0'); // possibly adding some extra null terminators to the end.
        }
    }
    return heap;
}

class CvlsStream1dArrayTest : public ::testing::TestWithParam<bool> {};

TEST_P(CvlsStream1dArrayTest, Basic) {
    const bool chunked = GetParam();
    size_t nlen = 12345;
    std::vector<std::string> example(nlen);
    for (size_t i = 0; i < nlen; ++i) {
        example[i] = std::to_string(i);
    }

    // Creating a file.
    const std::string path = "TEST-vls-stream.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        std::vector<ritsuko::cvls::Pointer<uint32_t, uint32_t> > pointers(nlen);
        size_t count = fill_pointers(example, pointers);
        auto dtype = ritsuko::cvls::define_pointer_datatype<uint32_t, uint32_t>();
        create_vls_pointer_dataset(handle, "foo", pointers, dtype, /* chunk_size = */ (chunked ? 51 : 0));

        auto heap = create_heap(example, count);
        create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
    }

    // Checking that the values are the same.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto phandle = handle.openDataSet("foo");
    auto chandle = handle.openDataSet("bar");

    ritsuko::cvls::Stream1dArray<std::uint64_t, std::uint64_t> stream(phandle, nlen, chandle);
    hsize_t total = 0;
    while (true) {
        hsize_t loaded = stream.load();
        EXPECT_EQ(total, stream.start());
        if (loaded == 0) {
            break;
        }

        auto chunk = stream.contents();
        for (hsize_t i = 0; i < loaded; ++i) {
            EXPECT_EQ(example[i + stream.start()], chunk[i]);
        }
        total += loaded;
    }

    EXPECT_EQ(total, nlen);
}

INSTANTIATE_TEST_SUITE_P(
    CvlsStream1dArray,
    CvlsStream1dArrayTest,
    ::testing::Values(false, true)
);

TEST(CvlsStream1dArray, NullTerminated) {
    size_t nlen = 1000;
    std::vector<std::string> example(nlen);
    std::mt19937_64 rng(999);
    for (size_t i = 0; i < nlen; ++i) {
        example[i] = std::to_string(rng());
    }

    // Creating a file.
    const std::string path = "TEST-vls-stream.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        size_t extra = 2; // injecting some extra null terminators, to check that we respect the first null.

        std::vector<ritsuko::cvls::Pointer<std::uint32_t, std::uint32_t> > pointers(nlen);
        size_t count = fill_pointers(example, pointers, extra);
        auto dtype = ritsuko::cvls::define_pointer_datatype<uint32_t, uint32_t>();
        create_vls_pointer_dataset(handle, "foo", pointers, dtype, /* chunk_size = */ 17);

        auto heap = create_heap(example, count, extra);
        create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto phandle = handle.openDataSet("foo");
    auto chandle = handle.openDataSet("bar");

    ritsuko::cvls::Stream1dArray<std::uint64_t, std::uint64_t> stream(phandle, nlen, chandle);
    while (true) {
        hsize_t loaded = stream.load();
        if (loaded == 0) {
            break;
        }
        auto chunk = stream.contents();
        for (hsize_t i = 0; i < loaded; ++i) {
            EXPECT_EQ(example[i + stream.start()], chunk[i]);
        }
    }
}

TEST(CvlsStream1dArray, Unicode) {
    std::vector<std::string> example { 
        "the value of π is around 3.1415926535",
        "alpha globulins consist of two principal fractions, α1 and α2",
        "😀😄😆🤣"
    };
    const auto nlen = example.size();

    // Creating a file.
    const std::string path = "TEST-vls-stream.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        std::vector<ritsuko::cvls::Pointer<std::uint32_t, std::uint32_t> > pointers(nlen);
        size_t count = fill_pointers(example, pointers);
        auto dtype = ritsuko::cvls::define_pointer_datatype<std::uint32_t, std::uint32_t>();
        create_vls_pointer_dataset(handle, "foo", pointers, dtype);

        auto heap = create_heap(example, count);
        create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
    }

    // Checking that the values are the same.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto phandle = handle.openDataSet("foo");
    auto chandle = handle.openDataSet("bar");

    ritsuko::cvls::Stream1dArray<uint64_t, uint64_t> stream(phandle, nlen, chandle);
    while (true) {
        hsize_t loaded = stream.load();
        if (loaded == 0) {
            break;
        }
        auto chunk = stream.contents();
        for (hsize_t i = 0; i < loaded; ++i) {
            EXPECT_EQ(example[i + stream.start()], chunk[i]);
        }
    }
}

TEST(CvlsStream1dArray, Failures) {
    const std::string path = "TEST-vls-stream.h5";

    // Start is out of range.
    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);

            std::vector<ritsuko::cvls::Pointer<std::uint32_t, std::uint32_t> > pointers(1);
            pointers[0].offset = 10;
            pointers[0].length = 0;
            auto dtype = ritsuko::cvls::define_pointer_datatype<std::uint32_t, std::uint32_t>();
            create_vls_pointer_dataset(handle, "foo", pointers, dtype);

            std::vector<unsigned char> heap;
            create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto phandle = handle.openDataSet("foo");
        auto chandle = handle.openDataSet("bar");
        ritsuko::cvls::Stream1dArray<std::uint64_t, std::uint64_t> stream(phandle, 1, chandle);

        std::string msg;
        try {
            stream.load();
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("out of range"));
    }

    // End is out of range.
    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);

            std::vector<ritsuko::cvls::Pointer<std::uint32_t, std::uint32_t> > pointers(1);
            pointers[0].offset = 0;
            pointers[0].length = 10;
            auto dtype = ritsuko::cvls::define_pointer_datatype<std::uint32_t, std::uint32_t>();
            create_vls_pointer_dataset(handle, "foo", pointers, dtype);

            std::vector<unsigned char> heap(5);
            create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto phandle = handle.openDataSet("foo");
        auto chandle = handle.openDataSet("bar");
        ritsuko::cvls::Stream1dArray<std::uint64_t, std::uint64_t> stream(phandle, 1, chandle);

        std::string msg;
        try {
            stream.load();
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("out of range"));
    }
}
