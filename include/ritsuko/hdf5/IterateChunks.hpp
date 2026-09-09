#ifndef RITSUKO_HDF5_ITERATE_CHUNKS_HPP
#define RITSUKO_HDF5_ITERATE_CHUNKS_HPP

#include "H5Cpp.h"

#include <vector>
#include <algorithm>
#include <cmath>

/**
 * @file IterateChunks.hpp
 * @brief Iterate through a dataspace by chunk.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @brief Iterate through an high-dimensional dataspace by chunk.
 *
 * This iterates through an N-dimensional dataspace in a chunkwise fashion,
 * typically to stream a chunked HDF5 dataset into memory.
 */
struct IterateChunks {
    /**
     * @param data_dimensions Dataset dimension extents.
     * @param chunk_dimensions Chunk dimensions.
     * Each entry may be less than, equal to, or greater than the corresponding entry of `data_dimensions`;
     * the only requirement is that, if `data_dimensions[i]` is positive, so is `chunk_dimensions[i]`.
     */
    IterateChunks(std::vector<hsize_t> data_dimensions, std::vector<hsize_t> chunk_dimensions) : 
        my_data_extent(std::move(data_dimensions)), 
        my_chunk_extent(std::move(chunk_dimensions)), 
        my_starts(my_data_extent.size()), 
        my_counts(my_data_extent.size())
    {
        const auto ndims = my_data_extent.size();
        assert(ndims == my_chunk_extent.size());

        std::size_t num_empty = 0;
        for (std::size_t d = 0; d < ndims; ++d) {
            my_chunk_extent[d] = std::min(my_data_extent[d], my_chunk_extent[d]);
            my_counts[d] = my_chunk_extent[d];
            num_empty += (my_chunk_extent[d] == 0); 
        }

        if (ndims == 0 || num_empty) {
            my_finished = true;
        } else {
            // So first advance has no effect.
            my_counts.back() = 0;
        }
    }

    /**
     * Advance to the next chunk. 
     * On return, this is now the "current" chunk.
     *
     * @return Whether the advance was successful.
     * If false, the iteration has finished.
     */
    bool advance() {
        if (my_finished) {
            return false;
        }

        // Prioritizing the last dimension as this is the fastest-changing.
        const auto ndims = my_data_extent.size();
        for (auto i = ndims; i > 0; --i) {
            const auto d = i - 1;
            my_starts[d] += my_counts[d];
            if (my_starts[d] < my_data_extent[d]) {
                my_counts[d] = std::min(my_data_extent[d] - my_starts[d], my_chunk_extent[d]);
                return true; 
            }

            my_starts[d] = 0;
            my_counts[d] = my_chunk_extent[d];
        }

        my_finished = true;
        return false;
    }

public:
    /**
     * This should only be called after a call to `advance()` returns true.
     *
     * @return Starting coordinates of the current chunk. 
     */
    const std::vector<hsize_t>& starts () const {
        return my_starts;
    }

    /**
     * This should only be called after a call to `advance()` returns true.
     *
     * @return Dimensions of the current chunk. 
     * This is usually equal to the chunk dimensions used in the constructor,
     * except at the edges of the dataset where the current chunk may be truncated.
     */
    const std::vector<hsize_t>& counts () const {
        return my_counts;
    }

    /**
     * @return Dimensions of the dataset, as provided in the constructor.
     */
    const std::vector<hsize_t>& data_dimensions() const {
        return my_data_extent;
    }

    /**
     * @return Dimensions of the chunks.
     * Each entry will be no greater than the corresponding entry of the `chunk_dimensions` in the constructor.
     * Each entry will be no smaller than the corresponding entry of `counts()` at any time.
     */
    const std::vector<hsize_t>& chunk_dimensions() const {
        return my_chunk_extent;
    }

private:
    std::vector<hsize_t> my_data_extent, my_chunk_extent;
    std::vector<hsize_t> my_starts, my_counts;
    bool my_finished = false;
};

}

}

#endif
