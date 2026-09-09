#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/IterateChunks.hpp"

class Hdf5IterateChunksTest : public ::testing::TestWithParam<std::tuple<std::vector<hsize_t>, std::vector<hsize_t> > > {};

TEST_P(Hdf5IterateChunksTest, Basic) {
    auto params = GetParam();
    auto dims = std::get<0>(params);
    auto block = std::get<1>(params);

    ritsuko::hdf5::IterateChunks handler(dims, block);
    EXPECT_EQ(handler.data_dimensions(), dims);
    EXPECT_EQ(handler.chunk_dimensions(), block);

    const std::size_t ndims = dims.size();
    std::size_t total = 1;
    for (std::size_t d = 0; d < ndims; ++d) {
        total *= dims[d];
    }
    std::vector<char> check(total);

    while (handler.advance()) {
        const auto& counts = handler.counts();
        const auto& starts = handler.starts();

        for (std::size_t d = 0; d < ndims; ++d) {
            EXPECT_EQ(starts[d] % block[d], 0);
            EXPECT_GE(starts[d], 0);
            EXPECT_GT(counts[d], 0);
            EXPECT_LE(counts[d], block[d]);
            EXPECT_LE(starts[d] + counts[d], dims[d]);
        }

        if (ndims == 1) {
            for (hsize_t i = starts[0], iend = starts[0] + counts[0]; i < iend; ++i) {
                check[i] = 1;
            }
        } else if (ndims == 2) {
            for (hsize_t i = starts[0], iend = starts[0] + counts[0]; i < iend; ++i) {
                for (hsize_t j = starts[1], jend = starts[1] + counts[1]; j < jend; ++j) {
                    check[i * dims[1] + j] = 1;
                }
            }
        } else if (ndims == 3) {
            for (hsize_t i = starts[0], iend = starts[0] + counts[0]; i < iend; ++i) {
                for (hsize_t j = starts[1], jend = starts[1] + counts[1]; j < jend; ++j) {
                    for (hsize_t k = starts[2], kend = starts[2] + counts[2]; k < kend; ++k) {
                        check[(i * dims[1] + j) * dims[2] + k] = 1;
                    }
                }
            }
        }
    }

    std::vector<char> expected(total, 1);
    EXPECT_EQ(check, expected);
    EXPECT_FALSE(handler.advance());
}

INSTANTIATE_TEST_SUITE_P(
    Hdf5IterateChunks,
    Hdf5IterateChunksTest,
    ::testing::Values(
        std::make_tuple(std::vector<hsize_t>{ 100 }, std::vector<hsize_t>{ 20 }),
        std::make_tuple(std::vector<hsize_t>{ 312 }, std::vector<hsize_t>{ 27 }),
        std::make_tuple(std::vector<hsize_t>{ 50, 100 }, std::vector<hsize_t>{ 5, 25 }),
        std::make_tuple(std::vector<hsize_t>{ 176, 99 }, std::vector<hsize_t>{ 31, 17 }),
        std::make_tuple(std::vector<hsize_t>{ 20, 50, 40 }, std::vector<hsize_t>{ 2, 10, 8 }),
        std::make_tuple(std::vector<hsize_t>{ 45, 13, 88 }, std::vector<hsize_t>{ 7, 6, 12 })
    )
);

TEST(Hdf5IterateChunks, Capped) {
    std::vector<hsize_t> dims { 20, 10, 30 };
    std::vector<hsize_t> block { 15, 20, 50 };
    ritsuko::hdf5::IterateChunks handler(dims, block);

    std::vector<hsize_t> expected { 15, 10, 30 };
    EXPECT_EQ(handler.chunk_dimensions(), expected);
}

TEST(Hdf5IterateChunks, Empty) {
    {
        std::vector<hsize_t> dims { 240, 0, 300 };
        std::vector<hsize_t> block { 29, 0, 77 };
        ritsuko::hdf5::IterateChunks handler(dims, block);
        EXPECT_FALSE(handler.advance());
    }

    {
        ritsuko::hdf5::IterateChunks handler({}, {});
        EXPECT_FALSE(handler.advance());
    }
}
