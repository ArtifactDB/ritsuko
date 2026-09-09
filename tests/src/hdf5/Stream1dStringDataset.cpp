#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/Stream1dStringDataset.hpp"
#include "ritsuko/hdf5/validate_string.hpp"
#include "utils.h"
#include <numeric>
#include <string>

class Hdf5Stream1dStringDatasetTest : public ::testing::TestWithParam<bool> {};

TEST_P(Hdf5Stream1dStringDatasetTest, Fixed) {
    const char* path = "TEST-load-string.h5";
    const bool chunked = GetParam();

    std::vector<std::string> example(11221);
    for (size_t i = 0; i < example.size(); ++i) {
        example[i] = std::to_string(i);
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, false, (chunked ? 444 : 0));
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    ritsuko::hdf5::Stream1dStringDataset stream(dhandle, example.size());

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

TEST_P(Hdf5Stream1dStringDatasetTest, Variable) {
    const char* path = "TEST-load-string.h5";
    const bool chunked = GetParam();

    std::vector<std::string> example(8877);
    for (size_t i = 0; i < example.size(); ++i) {
        example[i] = std::to_string(i);
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, true, (chunked ? 999 : 0));
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    ritsuko::hdf5::Stream1dStringDataset stream(dhandle, example.size());

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
    Hdf5Stream1dStringDataset, 
    Hdf5Stream1dStringDatasetTest, 
    ::testing::Values(false, true)
);

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
    ritsuko::hdf5::Stream1dStringDataset stream(dhandle, 10);
    std::string msg;
    try {
        stream.load();
    } catch (std::exception& e) {
        msg = e.what();
    }
    EXPECT_THAT(msg, ::testing::HasSubstr("NULL pointer"));
}
