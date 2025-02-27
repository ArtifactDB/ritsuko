#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstdint>
#include <random>

#include "ritsuko/hdf5/vls/Stream1dDataset.hpp"
#include "ritsuko/hdf5/vls/define_pointer_datatype.hpp"
#include "ritsuko/hdf5/vls/open.hpp"

#include "utils.h"
#include "../utils.h"

TEST(VlsStream1dDataset, Basic) {
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
        size_t count = 0;
        for (size_t i = 0; i < nlen; ++i) {
            pointers[i].start = count;
            auto ex_size = example[i].size();
            pointers[i].size = ex_size;
            count += ex_size;
        }
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        create_vls_pointer_dataset(handle, "foo", pointers, dtype, /* chunk_size = */ 51);

        std::vector<unsigned char> concatenated;
        concatenated.reserve(count);
        for (const auto& ex : example) {
            auto ptr = reinterpret_cast<const unsigned char*>(ex.c_str());
            concatenated.insert(concatenated.end(), ptr, ptr + ex.size());
        }
        create_dataset(handle, "bar", concatenated, H5::PredType::NATIVE_UINT8);
    }

    // Checking that the values are the same, with a few buffer sizes to check that iteration works correctly.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
    auto chandle = ritsuko::hdf5::vls::open_concatenated(handle, "bar");

    std::vector<size_t> buffer_sizes { 10, 200, 500 };
    for (size_t buffer_size : buffer_sizes) {
        ritsuko::hdf5::vls::Stream1dDataset<uint64_t, uint64_t> stream(&phandle, &chandle, buffer_size);
        EXPECT_EQ(stream.length(), example.size());
        for (auto x : example) {
            EXPECT_EQ(stream.get(), x);
            stream.next();
        }
    }
}

TEST(VlsStream1dDataset, NullTerminated) {
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

        std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > pointers(nlen);
        size_t count = 0;
        for (size_t i = 0; i < nlen; ++i) {
            pointers[i].start = count;
            auto ex_size = example[i].size() + 2; // adding some extra null terminators.
            pointers[i].size = ex_size;
            count += ex_size;
        }
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        create_vls_pointer_dataset(handle, "foo", pointers, dtype, /* chunk_size = */ 17);

        std::vector<unsigned char> concatenated;
        concatenated.reserve(count);
        for (const auto& ex : example) {
            auto ptr = reinterpret_cast<const unsigned char*>(ex.c_str());
            concatenated.insert(concatenated.end(), ptr, ptr + ex.size());
            concatenated.insert(concatenated.end(), 2, '\0');
        }
        create_dataset(handle, "bar", concatenated, H5::PredType::NATIVE_UINT8);
    }

    // Checking that the values are the same.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
    auto chandle = ritsuko::hdf5::vls::open_concatenated(handle, "bar");

    std::vector<size_t> buffer_sizes { 11, 39, 71};
    for (size_t buffer_size : buffer_sizes) {
        ritsuko::hdf5::vls::Stream1dDataset<uint64_t, uint64_t> stream(&phandle, &chandle, buffer_size);
        EXPECT_EQ(stream.length(), example.size());
        for (auto x : example) {
            EXPECT_EQ(stream.get(), x);
            stream.next();
        }
    }
}

TEST(VlsStream1dDataset, Unicode) {
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
        size_t count = 0;
        for (size_t i = 0; i < nlen; ++i) {
            pointers[i].start = count;
            auto ex_size = example[i].size(); 
            pointers[i].size = ex_size;
            count += ex_size;
        }
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        create_vls_pointer_dataset(handle, "foo", pointers, dtype);

        std::vector<unsigned char> concatenated;
        concatenated.reserve(count);
        for (const auto& ex : example) {
            auto ptr = reinterpret_cast<const unsigned char*>(ex.c_str());
            concatenated.insert(concatenated.end(), ptr, ptr + ex.size());
        }
        create_dataset(handle, "bar", concatenated, H5::PredType::NATIVE_UINT8);
    }

    // Checking that the values are the same.
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
    auto chandle = ritsuko::hdf5::vls::open_concatenated(handle, "bar");

    ritsuko::hdf5::vls::Stream1dDataset<uint64_t, uint64_t> stream(&phandle, &chandle, 200);
    EXPECT_EQ(stream.length(), example.size());
    for (auto x : example) {
        EXPECT_EQ(stream.get(), x);
        stream.next();
    }
}

TEST(VlsStream1dDataset, Failures) {
    const std::string path = "TEST-vls-stream.h5";

    // Start is out of range.
    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);

            std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > pointers(1);
            pointers[0].start = 10;
            pointers[0].size = 0;
            auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
            create_vls_pointer_dataset(handle, "foo", pointers, dtype);

            std::vector<unsigned char> concatenated;
            create_dataset(handle, "bar", concatenated, H5::PredType::NATIVE_UINT8);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
        auto chandle = ritsuko::hdf5::vls::open_concatenated(handle, "bar");
        ritsuko::hdf5::vls::Stream1dDataset<uint64_t, uint64_t> stream(&phandle, &chandle, 100);
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
            pointers[0].start = 0;
            pointers[0].size = 10;
            auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
            create_vls_pointer_dataset(handle, "foo", pointers, dtype);

            std::vector<unsigned char> concatenated(5);
            create_dataset(handle, "bar", concatenated, H5::PredType::NATIVE_UINT8);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
        auto chandle = ritsuko::hdf5::vls::open_concatenated(handle, "bar");
        ritsuko::hdf5::vls::Stream1dDataset<uint64_t, uint64_t> stream(&phandle, &chandle, 100);
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
            pointers[0].start = 0;
            pointers[0].size = 0;
            auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
            create_vls_pointer_dataset(handle, "foo", pointers, dtype);

            std::vector<unsigned char> concatenated;
            create_dataset(handle, "bar", concatenated, H5::PredType::NATIVE_UINT8);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "foo", 64, 64);
        auto chandle = ritsuko::hdf5::vls::open_concatenated(handle, "bar");
        ritsuko::hdf5::vls::Stream1dDataset<uint64_t, uint64_t> stream(&phandle, &chandle, 100);
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
