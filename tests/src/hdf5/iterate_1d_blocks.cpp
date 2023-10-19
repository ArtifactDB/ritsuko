#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/iterate_1d_blocks.hpp"
#include "ritsuko/hdf5/pick_1d_block_size.hpp"
#include "utils.h"
#include <numeric>

TEST(Hdf5Iterate1dBlocks, Basic) {
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
    for (auto buf : buffer_sizes) {
        auto dhandle = handle.openDataSet("foobar");
        auto block_size = ritsuko::hdf5::pick_1d_block_size(dhandle.getCreatePlist(), example.size(), buf);
        std::vector<int> buffer(example.size());
        ritsuko::hdf5::iterate_1d_blocks(
            example.size(), 
            block_size, 
            [&](hsize_t start, hsize_t, const H5::DataSpace& memspace, const H5::DataSpace& dataspace) {
                dhandle.read(buffer.data() + start, H5::PredType::NATIVE_INT, memspace, dataspace);
            }
        );
        EXPECT_EQ(buffer, example);
    }
}
