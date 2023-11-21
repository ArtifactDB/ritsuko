#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/Numeric1dDatasetStream.hpp"
#include "utils.h"
#include <numeric>

TEST(Hdf5Numeric1dDatasetStream, Basic) {
    const char* path = "TEST-iterate.h5";

    std::vector<int> example(29726);
    std::iota(example.begin(), example.end(), 0);

    hsize_t block_size = 471;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, H5::PredType::NATIVE_INT, block_size);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    std::vector<int> buffer_sizes { 100, 1000, 10000, 100000 };

    // One value at a time.
    for (auto buf : buffer_sizes) {
        auto dhandle = handle.openDataSet("foobar");
        ritsuko::hdf5::Numeric1dDatasetStream<int> stream(&dhandle, buf);
        EXPECT_EQ(stream.length(), example.size());
        for (auto x : example) {
            EXPECT_EQ(stream.get(), x);
            stream.next();
        }
    }

    // Fetching a data block.
    for (auto buf : buffer_sizes) {
        auto dhandle = handle.openDataSet("foobar");
        ritsuko::hdf5::Numeric1dDatasetStream<int> stream(&dhandle, buf);

        size_t start = 0;
        while (start < example.size()) {
            auto many = stream.get_many();
            for (size_t i = 0; i < many.second; ++i) {
                EXPECT_EQ(example[i + start], many.first[i]);
            }
            start += many.second;
            stream.next(many.second);
        }

        EXPECT_EQ(start, example.size());
    }
}

TEST(Hdf5Numeric1dDatasetStream, Floats) {
    const char* path = "TEST-iterate.h5";

    // Works with floating-point data.
    std::vector<double> example(10000);
    std::iota(example.begin(), example.end(), 0.5);
    hsize_t block_size = 57;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, H5::PredType::NATIVE_DOUBLE, block_size);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    ritsuko::hdf5::Numeric1dDatasetStream<double> stream(&dhandle, 100);
    EXPECT_EQ(stream.length(), example.size());

    for (auto x : example) {
        EXPECT_EQ(stream.get(), x);
        stream.next();
    }
}
