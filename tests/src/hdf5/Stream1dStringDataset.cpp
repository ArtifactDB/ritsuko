#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/Stream1dStringDataset.hpp"
#include "ritsuko/hdf5/validate_string.hpp"
#include "utils.h"
#include <numeric>
#include <string>

TEST(Hdf5Stream1dStringDataset, Fixed) {
    const char* path = "TEST-load-string.h5";

    std::vector<std::string> example(11221);
    for (size_t i = 0; i < example.size(); ++i) {
        example[i] = std::to_string(i);
    }

    hsize_t chunk_size = 471;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, false, chunk_size);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    std::vector<int> buffer_sizes { 100, 1000, 10000, 100000 };

    // Getting.
    for (auto buf : buffer_sizes) {
        ritsuko::hdf5::Stream1dStringDataset stream(&dhandle, buf);
        EXPECT_EQ(stream.length(), example.size());
        for (auto x : example) {
            EXPECT_EQ(stream.get(), x);
            stream.next();
        }
    }

    // Stealing.
    for (auto buf : buffer_sizes) {
        ritsuko::hdf5::Stream1dStringDataset stream(&dhandle, buf);
        EXPECT_EQ(stream.length(), example.size());
        for (auto x : example) {
            EXPECT_EQ(stream.steal(), x);
            stream.next();
        }
    }
}

TEST(Hdf5Stream1dStringDataset, Variable) {
    const char* path = "TEST-load-string.h5";

    std::vector<std::string> example(8877);
    for (size_t i = 0; i < example.size(); ++i) {
        example[i] = std::to_string(i);
    }

    hsize_t chunk_size = 999;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, true, chunk_size);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    std::vector<int> buffer_sizes { 100, 1000, 10000, 100000 };

    // Getting.
    for (auto buf : buffer_sizes) {
        ritsuko::hdf5::Stream1dStringDataset stream(&dhandle, buf);
        EXPECT_EQ(stream.length(), example.size());
        for (auto x : example) {
            EXPECT_EQ(stream.get(), x);
            stream.next();
        }
    }

    // Stealing.
    for (auto buf : buffer_sizes) {
        ritsuko::hdf5::Stream1dStringDataset stream(&dhandle, buf);
        EXPECT_EQ(stream.length(), example.size());
        for (auto x : example) {
            EXPECT_EQ(stream.steal(), x);
            stream.next();
        }
    }

    // Validating.
    for (auto buf : buffer_sizes) {
        ritsuko::hdf5::validate_1d_string_dataset(dhandle, buf);
    }
}

TEST(Hdf5Stream1dStringDataset, VariableNullFail) {
    const char* path = "TEST-load-string.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        hsize_t len = 10;
        H5::DataSpace dspace(1, &len);
        handle.createDataSet("foobar", H5::StrType(0, H5T_VARIABLE), dspace);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");

    ritsuko::hdf5::Stream1dStringDataset stream(&dhandle, 100);
    EXPECT_ANY_THROW({
        try {
            stream.get();
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("NULL pointer"));
            throw;
        }
    });

    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::validate_1d_string_dataset(dhandle, 100);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("NULL pointer"));
            throw;
        }
    });
}
