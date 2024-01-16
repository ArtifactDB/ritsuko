#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/pick_1d_block_size.hpp"

TEST(Hdf5Pick1dBlockSize, Compressed) {
    hsize_t buffer = 10000;

    // Rounds down to the nearest multiple.
    hsize_t len = 30000;
    hsize_t chunk = 57;

    {
        H5::DSetCreatPropList cplist;
        cplist.setChunk(1, &chunk);
        cplist.setDeflate(8);
        auto block_size = ritsuko::hdf5::pick_1d_block_size(cplist, len, buffer);
        EXPECT_EQ(block_size, (buffer / chunk) * chunk);
        EXPECT_TRUE(block_size > chunk);
        EXPECT_TRUE(block_size < len);
    }

    // Or uses the entire chunk.
    {
        H5::DSetCreatPropList cplist;
        hsize_t chunk = 15000;
        cplist.setChunk(1, &chunk);
        cplist.setDeflate(8);
        auto block_size = ritsuko::hdf5::pick_1d_block_size(cplist, len, buffer);
        EXPECT_EQ(block_size, chunk);
    }
}

TEST(Hdf5Pick1dBlockSize, Uncompressed) {
    hsize_t buffer = 10000;
    hsize_t len = 13232;
    {
        H5::DSetCreatPropList cplist;
        auto block_size = ritsuko::hdf5::pick_1d_block_size(cplist, len, buffer);
        EXPECT_EQ(block_size, buffer);
    }
}

TEST(Hdf5Pick1dBlockSize, Short) {
    hsize_t buffer = 10000;

    hsize_t shortlen = 5;
    {
        H5::DSetCreatPropList cplist;
        auto block_size = ritsuko::hdf5::pick_1d_block_size(cplist, shortlen, buffer);
        EXPECT_EQ(block_size, shortlen);
    }

    {
        H5::DSetCreatPropList cplist;
        auto block_size = ritsuko::hdf5::pick_1d_block_size(cplist, 0, buffer);
        EXPECT_EQ(block_size, 0);
    }
}
