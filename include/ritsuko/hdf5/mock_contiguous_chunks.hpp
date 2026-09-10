#ifndef RITSUKO_HDF5_MOCK_CONTIGUOUS_CHUNKS_HPP
#define RITSUKO_HDF5_MOCK_CONTIGUOUS_CHUNKS_HPP

#include "H5Cpp.h"

/**
 * @file mock_contiguous_chunks.hpp
 * @brief Mock chunk sizes for a contiguous HDF5 dataset.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * Mock chunk dimensions for a contiguous dataset.
 * This allows classes like `IterateBlock` to be applied to contiguous datasets.
 *
 * @param dimensions Array of the dataset dimensions.
 * @param chunk_size Size of the mock chunk, in terms of the number of dimensions.
 *
 * @return Chunk dimensions.
 * This is of the same length as `dimensions`.
 * Each entry is no greater than the corresponding element of `dimensions`,
 * unless the latter is zero in which case the former is set to 1.
 */
inline std::vector<hsize_t> mock_contiguous_chunks(const std::vector<hsize_t>& dimensions, hsize_t chunk_size) {
    const auto ndims = dimensions.size();
    std::vector<hsize_t> output(ndims, 1);
    // Starting from the end, as this is the fastest-changing.
    for (hsize_t i = ndims; i > 0; --i) {
        const auto d = i - 1;
        if (dimensions[d] == 0) {
            continue;
        }
        if (chunk_size <= dimensions[d]) {
            output[d] = chunk_size;
            break;
        }
        output[d] = dimensions[d];
        chunk_size /= dimensions[d];
    }
    return output;
}

}

}

#endif
