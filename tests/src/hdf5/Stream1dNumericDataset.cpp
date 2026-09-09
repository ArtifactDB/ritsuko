#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/Stream1dNumericDataset.hpp"
#include "utils.h"
#include <numeric>

class Hdf5Stream1dNumericDatasetTest : public ::testing::TestWithParam<bool> {};

TEST_P(Hdf5Stream1dNumericDatasetTest, Integer) {
    const char* path = "TEST-iterate.h5";
    const bool chunked = GetParam();

    std::vector<int> example(29726);
    std::iota(example.begin(), example.end(), 0);

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, H5::PredType::NATIVE_INT, (chunked ? 471 : 0));
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");

    ritsuko::hdf5::Stream1dNumericDataset<int> stream(dhandle, example.size());
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

    EXPECT_EQ(total, example.size());
}

TEST_P(Hdf5Stream1dNumericDatasetTest, Float) {
    const char* path = "TEST-iterate.h5";
    const bool chunked = GetParam();

    // Works with floating-point data.
    std::vector<double> example(10000);
    std::iota(example.begin(), example.end(), 0.5);

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, H5::PredType::NATIVE_DOUBLE, (chunked ? 57 : 0));
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");

    ritsuko::hdf5::Stream1dNumericDataset<double> stream(dhandle, example.size());
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

INSTANTIATE_TEST_SUITE_P(
    Hdf5Stream1dNumericDataset,
    Hdf5Stream1dNumericDatasetTest,
    ::testing::Values(false, true)
);
