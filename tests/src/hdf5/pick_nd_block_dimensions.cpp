#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/pick_nd_block_dimensions.hpp"

TEST(Hdf5PickNdBlockSize, Compressed2D) {
    std::vector<hsize_t> dims { 2400, 3000 };
    hsize_t chunk [2] = { 57, 13 };

    H5::DSetCreatPropList cplist;
    cplist.setChunk(2, chunk);
    cplist.setDeflate(8);

    // Rounds down to the nearest multiple, without overflow along earlier dimensions.
    {
        hsize_t buffer = 10000;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 2);
        EXPECT_EQ(block_dims[0], chunk[0]);
        EXPECT_EQ(block_dims[1], (buffer / (chunk[0] * chunk[1])) * chunk[1]);
        EXPECT_NE(block_dims[1], dims[1]);
        EXPECT_NE(block_dims[1], chunk[1]);
    }

    // Rounds down to the nearest multiple, now overflowing to the next dimension.
    {
        hsize_t buffer = 1000000;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 2);
        EXPECT_NE(block_dims[0], dims[0]);
        EXPECT_NE(block_dims[0], chunk[0]);
        EXPECT_EQ(block_dims[0], (buffer / (dims[1] * chunk[0])) * chunk[0]);
        EXPECT_EQ(block_dims[1], dims[1]);
    }

    // Uses everything.
    {
        hsize_t buffer = 1000000000;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 2);
        EXPECT_EQ(block_dims[0], dims[0]);
        EXPECT_EQ(block_dims[1], dims[1]);
    }

    // Uses a single chunk.
    {
        hsize_t buffer = 1;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 2);
        EXPECT_EQ(block_dims[0], chunk[0]);
        EXPECT_EQ(block_dims[1], chunk[1]);
    }
}

TEST(Hdf5PickNdBlockSize, Compressed3D) {
    std::vector<hsize_t> dims { 2400, 1000, 3000 };
    hsize_t chunk [3] = { 23, 11, 9 };

    H5::DSetCreatPropList cplist;
    cplist.setChunk(3, chunk);
    cplist.setDeflate(8);

    // Rounds down to the nearest multiple, without overflow along earlier dimensions.
    {
        hsize_t buffer = 10000;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 3);
        EXPECT_EQ(block_dims[0], chunk[0]);
        EXPECT_EQ(block_dims[1], chunk[1]);
        EXPECT_EQ(block_dims[2], (buffer / (chunk[0] * chunk[1] * chunk[2])) * chunk[2]);
        EXPECT_NE(block_dims[2], dims[2]);
        EXPECT_NE(block_dims[2], chunk[2]);
    }

    // Rounds down to the nearest multiple, now overflowing to the next dimension.
    {
        hsize_t buffer = 10000000;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 3);
        EXPECT_EQ(block_dims[0], chunk[0]);
        EXPECT_EQ(block_dims[1], (buffer / (chunk[0] * chunk[1] * dims[2])) * chunk[1]);
        EXPECT_NE(block_dims[1], dims[1]);
        EXPECT_NE(block_dims[1], chunk[1]);
        EXPECT_EQ(block_dims[2], dims[2]);
    }

    // Rounds down to the nearest multiple, now overflowing to the first dimension.
    {
        hsize_t buffer = 1000000000;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 3);
        EXPECT_EQ(block_dims[0], (buffer / (chunk[0] * dims[1] * dims[2])) * chunk[0]);
        EXPECT_NE(block_dims[0], dims[0]);
        EXPECT_NE(block_dims[0], chunk[0]);
        EXPECT_EQ(block_dims[1], dims[1]);
        EXPECT_EQ(block_dims[2], dims[2]);
    }
}

TEST(Hdf5PickNdBlockSize, Uncompressed) {
    std::vector<hsize_t> dims { 2400, 3000 };
    H5::DSetCreatPropList cplist;

    {
        hsize_t buffer = 10000;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 2);
        EXPECT_EQ(block_dims[0], buffer / dims[1]);
        EXPECT_EQ(block_dims[1], dims[1]);
    }

    {
        hsize_t buffer = 1000;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 2);
        EXPECT_EQ(block_dims[0], 1);
        EXPECT_EQ(block_dims[1], buffer);
    }

    {
        hsize_t buffer = 1000000000;
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, buffer);
        EXPECT_EQ(block_dims.size(), 2);
        EXPECT_EQ(block_dims[0], dims[0]);
        EXPECT_EQ(block_dims[1], dims[1]);
    }

    // Works for zero-dimension datasets.
    {
        std::vector<hsize_t> dims { 2400, 0, 3000 };
        auto block_dims = ritsuko::hdf5::pick_nd_block_dimensions(cplist, dims, 10000);
        EXPECT_EQ(block_dims.size(), 3);
        EXPECT_EQ(block_dims[0], 0);
        EXPECT_EQ(block_dims[1], 0);
        EXPECT_EQ(block_dims[2], 0);
    }
}
