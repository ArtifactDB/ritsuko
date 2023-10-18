#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/pick_1d_block_size.hpp"

TEST(Hdf5Pick1dBlockSize, Compressed) {
    const char* path = "TEST-blocks.h5";
    hsize_t buffer = 10000;

    // Rounds down to the nearest multiple.
    hsize_t len = 30000;
    hsize_t chunk = 57;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace(1, &len);
        H5::DSetCreatPropList cplist;
        cplist.setChunk(1, &chunk);
        cplist.setDeflate(8);
        handle.createDataSet("YAY", H5::PredType::NATIVE_UINT8, dspace, cplist);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("YAY");
        auto block_size = ritsuko::hdf5::pick_1d_block_size(dhandle.getCreatePlist(), len, buffer);
        EXPECT_EQ(block_size, (buffer / chunk) * chunk);
    }

    // Or uses the entire chunk.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace(1, &len);
        H5::DSetCreatPropList cplist;
        hsize_t chunk = 15000;
        cplist.setChunk(1, &chunk);
        cplist.setDeflate(8);
        handle.createDataSet("YAY", H5::PredType::NATIVE_UINT8, dspace, cplist);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("YAY");
        auto block_size = ritsuko::hdf5::pick_1d_block_size(dhandle.getCreatePlist(), len, buffer);
        EXPECT_EQ(block_size, 15000);
    }
}

TEST(Hdf5Pick1dBlockSize, Uncompressed) {
    const char* path = "TEST-blocks.h5";
    hsize_t buffer = 10000;

    hsize_t len = 13232;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace(1, &len);
        handle.createDataSet("YAY", H5::PredType::NATIVE_UINT8, dspace);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("YAY");
        auto block_size = ritsuko::hdf5::pick_1d_block_size(dhandle.getCreatePlist(), len);
        EXPECT_EQ(block_size, buffer);
    }
}

TEST(Hdf5Pick1dBlockSize, Short) {
    const char* path = "TEST-blocks.h5";
    hsize_t buffer = 10000;

    hsize_t shortlen = 5;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace(1, &shortlen);
        handle.createDataSet("YAY", H5::PredType::NATIVE_UINT8, dspace);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("YAY");
        auto block_size = ritsuko::hdf5::pick_1d_block_size(dhandle.getCreatePlist(), shortlen, buffer);
        EXPECT_EQ(block_size, shortlen);
    }
}
