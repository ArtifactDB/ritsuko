#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstdint>
#include <random>

#include "ritsuko/hdf5/vls/Stream1dArray.hpp"
#include "ritsuko/hdf5/vls/Pointer.hpp"
#include "ritsuko/hdf5/vls/open.hpp"

#include "utils.h"
#include "../utils.h"

static size_t fill_pointers(const std::vector<std::string>& example, std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> >& pointers, size_t extra = 0) {
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

TEST(VlsStream1dArray, Basic) {
    size_t nlen = 12345;
    std::vector<std::string> example(nlen);
    for (size_t i = 0; i < nlen; ++i) {
        example[i] = std::to_string(i);
    }

    // Creating a file.
    const std::string path = "TEST-vls-stream.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > pointers(nlen);
        size_t count = fill_pointers(example, pointers);
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        create_vls_pointer_dataset(handle, "foo", pointers, dtype, /* chunk_size = */ 51);

        auto heap = create_heap(example, count);
        create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
    }

    // Checking that the values are the same, with a few buffer sizes to check that iteration works correctly.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
    auto chandle = ritsuko::hdf5::vls::open_heap(handle, "bar");

    std::vector<size_t> buffer_sizes { 10, 200, 500 };
    for (size_t buffer_size : buffer_sizes) {
        ritsuko::hdf5::vls::Stream1dArray<uint64_t, uint64_t> stream(&phandle, &chandle, buffer_size);
        EXPECT_EQ(stream.length(), example.size());
        for (auto x : example) {
            EXPECT_EQ(stream.get(), x);
            stream.next();
        }
    }
}

TEST(VlsStream1dArray, NullTerminated) {
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
        size_t extra = 2;

        std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > pointers(nlen);
        size_t count = fill_pointers(example, pointers, extra);
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        create_vls_pointer_dataset(handle, "foo", pointers, dtype, /* chunk_size = */ 17);

        auto heap = create_heap(example, count, extra);
        create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
    }

    // Checking that the values are the same. This time we use 'steal' just to get some coverage.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
    auto chandle = ritsuko::hdf5::vls::open_heap(handle, "bar");

    std::vector<size_t> buffer_sizes { 11, 39, 71 };
    for (size_t buffer_size : buffer_sizes) {
        ritsuko::hdf5::vls::Stream1dArray<uint64_t, uint64_t> stream(&phandle, &chandle, buffer_size);
        EXPECT_EQ(stream.length(), example.size());
        for (auto x : example) {
            EXPECT_EQ(stream.steal(), x);
            stream.next();
        }
    }
}

TEST(VlsStream1dArray, Unicode) {
    std::vector<std::string> example { 
        "the value of π is around 3.1415926535",
        "alpha globulins consist of two principal fractions, α1 and α2",
        "😀😄😆🤣"
    };
    size_t nlen = example.size();

    // Creating a file.
    const std::string path = "TEST-vls-stream.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);

        std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > pointers(nlen);
        size_t count = fill_pointers(example, pointers);
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        create_vls_pointer_dataset(handle, "foo", pointers, dtype);

        auto heap = create_heap(example, count);
        create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
    }

    // Checking that the values are the same.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
    auto chandle = ritsuko::hdf5::vls::open_heap(handle, "bar");

    ritsuko::hdf5::vls::Stream1dArray<uint64_t, uint64_t> stream(&phandle, &chandle, 200);
    EXPECT_EQ(stream.length(), example.size());
    for (auto x : example) {
        EXPECT_EQ(stream.get(), x);
        stream.next();
    }
}

TEST(VlsStream1dArray, Failures) {
    const std::string path = "TEST-vls-stream.h5";

    // Start is out of range.
    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);

            std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > pointers(1);
            pointers[0].offset = 10;
            pointers[0].length = 0;
            auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
            create_vls_pointer_dataset(handle, "foo", pointers, dtype);

            std::vector<unsigned char> heap;
            create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
        auto chandle = ritsuko::hdf5::vls::open_heap(handle, "bar");
        ritsuko::hdf5::vls::Stream1dArray<uint64_t, uint64_t> stream(&phandle, &chandle, 100);
        EXPECT_ANY_THROW({
            try {
                stream.get();
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("out of range"));
                throw;
            }
        });
    }

    // End is out of range.
    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);

            std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > pointers(1);
            pointers[0].offset = 0;
            pointers[0].length = 10;
            auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
            create_vls_pointer_dataset(handle, "foo", pointers, dtype);

            std::vector<unsigned char> heap(5);
            create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
        auto chandle = ritsuko::hdf5::vls::open_heap(handle, "bar");
        ritsuko::hdf5::vls::Stream1dArray<uint64_t, uint64_t> stream(&phandle, &chandle, 100);
        EXPECT_ANY_THROW({
            try {
                stream.get();
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("out of range"));
                throw;
            }
        });
    }

    // Too many requests.
    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);

            std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > pointers(1);
            pointers[0].offset = 0;
            pointers[0].length = 0;
            auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
            create_vls_pointer_dataset(handle, "foo", pointers, dtype);

            std::vector<unsigned char> heap;
            create_dataset(handle, "bar", heap, H5::PredType::NATIVE_UINT8);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
        auto chandle = ritsuko::hdf5::vls::open_heap(handle, "bar");
        ritsuko::hdf5::vls::Stream1dArray<uint64_t, uint64_t> stream(&phandle, &chandle, 100);
        EXPECT_EQ(stream.get(), std::string());
        stream.next();

        EXPECT_ANY_THROW({
            try {
                stream.get();
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("beyond the end"));
                throw;
            }
        });
    }
}
