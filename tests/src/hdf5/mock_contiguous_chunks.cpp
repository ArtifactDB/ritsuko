#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/mock_contiguous_chunks.hpp"

TEST(Hdf5MockContiguousChunks, NonEmpty) {
    std::vector<hsize_t> dims{ 12, 42, 35 };

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 10);
        std::vector<hsize_t> expected { 1, 1, 10 };
        EXPECT_EQ(mocked, expected);  
    }

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 50);
        std::vector<hsize_t> expected { 1, 1, 35 };
        EXPECT_EQ(mocked, expected);  
    }

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 150);
        std::vector<hsize_t> expected { 1, 4, 35 };
        EXPECT_EQ(mocked, expected);  
    }

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 1000);
        std::vector<hsize_t> expected { 1, 28, 35 };
        EXPECT_EQ(mocked, expected);  
    }

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 10000);
        std::vector<hsize_t> expected { 6, 42, 35 };
        EXPECT_EQ(mocked, expected);  
    }

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 100000);
        std::vector<hsize_t> expected { 12, 42, 35 };
        EXPECT_EQ(mocked, expected);  
    }
}

TEST(Hdf5MockContiguousChunks, Empty) {
    {
        std::vector<hsize_t> dims{ 12, 42, 0 };
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 10);
        std::vector<hsize_t> expected { 1, 10, 1 };
        EXPECT_EQ(mocked, expected);  
    }

    {
        std::vector<hsize_t> dims{ 12, 0, 42 };
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 50);
        std::vector<hsize_t> expected { 1, 1, 42 };
        EXPECT_EQ(mocked, expected);  
    }

    {
        std::vector<hsize_t> dims{ 12, 0, 42 };
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 500);
        std::vector<hsize_t> expected { 11, 1, 42 };
        EXPECT_EQ(mocked, expected);  
    }

    {
        std::vector<hsize_t> dims{ 0, 5, 10 };
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 150);
        std::vector<hsize_t> expected { 1, 5, 10 };
        EXPECT_EQ(mocked, expected);  
    }
}

TEST(Hdf5MockContiguousChunks, TwoDim) {
    std::vector<hsize_t> dims{ 12, 42 };

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 10);
        std::vector<hsize_t> expected { 1, 10 };
        EXPECT_EQ(mocked, expected);
    }

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 100);
        std::vector<hsize_t> expected { 2, 42 };
        EXPECT_EQ(mocked, expected);
    }

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 1000);
        std::vector<hsize_t> expected { 12, 42 };
        EXPECT_EQ(mocked, expected);
    }

    // Empty.
    {
        std::vector<hsize_t> dims{ 12, 0 };
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 20);
        std::vector<hsize_t> expected { 12, 1 };
        EXPECT_EQ(mocked, expected);
    }

    {
        std::vector<hsize_t> dims{ 0, 10 };
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 20);
        std::vector<hsize_t> expected { 1, 10 };
        EXPECT_EQ(mocked, expected);
    }
}

TEST(Hdf5MockContiguousChunks, OneDim) {
    std::vector<hsize_t> dims{ 42 };

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 10);
        std::vector<hsize_t> expected { 10 };
        EXPECT_EQ(mocked, expected);
    }

    {
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 100);
        std::vector<hsize_t> expected { 42 };
        EXPECT_EQ(mocked, expected);
    }

    // Empty.
    {
        std::vector<hsize_t> dims{ 0 };
        auto mocked = ritsuko::hdf5::mock_contiguous_chunks(dims, 20);
        std::vector<hsize_t> expected { 1 };
        EXPECT_EQ(mocked, expected);
    }
}
