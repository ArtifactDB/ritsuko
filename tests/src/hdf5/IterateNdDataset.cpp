#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/IterateNdDataset.hpp"

TEST(Hdf5IterateNdDataset, TwoDimensional) {
    std::vector<hsize_t> dims { 2400, 3000 };
    std::vector<hsize_t> block { 101, 55 };

    ritsuko::hdf5::IterateNdDataset handler(dims, block);
    EXPECT_EQ(handler.dimensions(), dims);
    EXPECT_EQ(handler.block_dimensions(), block);
    size_t total_iterated = 0;
    std::vector<hsize_t> test_start(2), test_end(2), test_empty(2);

    while (!handler.finished()) {
        total_iterated += handler.current_block_size();

        const auto& counts = handler.counts();
        const auto& starts = handler.starts();
        for (size_t d = 0; d < 2; ++d) {
            EXPECT_EQ(starts[d] % block[d], 0);
            EXPECT_TRUE(counts[d] > 0);
            EXPECT_TRUE(counts[d] <= block[d]);
        }

        size_t current_size = 1;
        for (auto b : counts) {
            current_size *= b;
        }
        EXPECT_EQ(handler.current_block_size(), current_size);

        EXPECT_EQ(handler.file_space().getSelectNpoints(), current_size);
        handler.file_space().getSelectBounds(test_start.data(), test_end.data());
        EXPECT_EQ(test_start, starts);
        auto ends = starts;
        for (size_t i = 0; i < 2; ++i) {
            ends[i] += counts[i] - 1;
        }
        EXPECT_EQ(test_end, ends);

        EXPECT_EQ(handler.memory_space().getSelectNpoints(), current_size);
        handler.memory_space().getSelectBounds(test_start.data(), test_end.data());
        ends = counts;
        for (auto& e : ends) { --e; }
        EXPECT_EQ(test_start, test_empty);
        EXPECT_EQ(test_end, ends);

        handler.next();
    }

    EXPECT_EQ(total_iterated, dims[0] * dims[1]);
}

TEST(Hdf5IterateNdDataset, ThreeDimensional) {
    std::vector<hsize_t> dims { 240, 100, 300 };
    std::vector<hsize_t> block { 29, 17, 77 };

    ritsuko::hdf5::IterateNdDataset handler(dims, block);
    size_t total_iterated = 0;
    std::vector<hsize_t> test_start(3), test_end(3), test_empty(3);

    while (!handler.finished()) {
        total_iterated += handler.current_block_size();

        const auto& counts = handler.counts();
        const auto& starts = handler.starts();
        for (size_t d = 0; d < 3; ++d) {
            EXPECT_EQ(starts[d] % block[d], 0);
            EXPECT_TRUE(counts[d] > 0);
            EXPECT_TRUE(counts[d] <= block[d]);
        }

        size_t current_size = 1;
        for (auto b : counts) {
            current_size *= b;
        }
        EXPECT_EQ(handler.current_block_size(), current_size);

        EXPECT_EQ(handler.file_space().getSelectNpoints(), current_size);
        handler.file_space().getSelectBounds(test_start.data(), test_end.data());
        EXPECT_EQ(test_start, starts);
        auto ends = starts;
        for (size_t i = 0; i < 3; ++i) {
            ends[i] += counts[i] - 1;
        }
        EXPECT_EQ(test_end, ends);

        EXPECT_EQ(handler.memory_space().getSelectNpoints(), current_size);
        handler.memory_space().getSelectBounds(test_start.data(), test_end.data());
        EXPECT_EQ(test_start, test_empty);
        ends = counts;
        for (auto& e : ends) { --e; }
        EXPECT_EQ(test_end, ends);

        handler.next();
    }

    EXPECT_EQ(total_iterated, dims[0] * dims[1] * dims[2]);
}

TEST(Hdf5IterateNdDataset, Empty) {
    std::vector<hsize_t> dims { 240, 0, 300 };
    std::vector<hsize_t> block { 29, 0, 77 };
    ritsuko::hdf5::IterateNdDataset handler(dims, block);
    EXPECT_TRUE(handler.finished());
}

TEST(Hdf5IterateNdDataset, Extraction) {
    std::vector<hsize_t> dims { 240, 100, 170 };
    std::vector<hsize_t> chunk{ 29, 17, 37 };
    std::vector<int> values(dims[0] * dims[1] * dims[2]);
    for (int i = 0, end = values.size(); i < end; ++i) {
        values[i] = i;
    }

    const char* path = "TEST-nd-iterate.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DSetCreatPropList cplist;
        cplist.setChunk(3, chunk.data());
        H5::DataSpace dspace(3, dims.data());
        auto dhandle = handle.createDataSet("foo", H5::PredType::NATIVE_INT32, dspace, cplist);
        dhandle.write(values.data(), H5::PredType::NATIVE_INT);
    }

    H5::H5File handle(path, H5F_ACC_RDWR);
    auto dhandle = handle.openDataSet("foo");
    auto block = chunk;
    for (auto& b : block) { b *= 2; }

    ritsuko::hdf5::IterateNdDataset iter(dims, block);
    std::vector<int> staging_ground(values.size());
    auto ptr = staging_ground.data();
    while (!iter.finished()) {
        dhandle.read(ptr, H5::PredType::NATIVE_INT, iter.memory_space(), iter.file_space());
        ptr += iter.current_block_size();
        iter.next();
    }

    std::sort(staging_ground.begin(), staging_ground.end());
    EXPECT_EQ(staging_ground, values);
}
