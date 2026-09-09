#ifndef RITSUKO_HDF5_STREAM_1D_NUMERIC_DATASET_HPP
#define RITSUKO_HDF5_STREAM_1D_NUMERIC_DATASET_HPP

#include "H5Cpp.h"

#include <vector>
#include <stdexcept>

#include "get_name.hpp"
#include "as_numeric_datatype.hpp"

/**
 * @file Stream1dNumericDataset.hpp
 * @brief Stream a numeric 1-dimensional HDF5 dataset into memory.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @brief Stream a 1-dimensional HDF5 numeric dataset into memory.
 * @tparam Type_ Type to represent the data in memory.
 *
 * This streams in a 1-dimensional HDF5 numeric dataset in a chunk-wise manner.
 * The aim is to enable inspection of the dataset contents while minimizing memory usage.
 */
template<typename Type_>
class Stream1dNumericDataset {
public:
    /**
     * @param data A 1-dimensional HDF5 numeric dataset.
     * @param length Length of the dataset as a 1-dimensional vector.
     */
    Stream1dNumericDataset(const H5::DataSet& data, hsize_t length) : 
        my_data(data), 
        my_full_length(length), 
        my_block_size([&]{
            const auto& plist = my_data.getCreatePlist();
            if (plist.getLayout() == H5D_CHUNKED) {
                hsize_t output;
                plist.getChunk(1, &output);
                return output;
            } else {
                // Hard-coding the mock chunk size for contiguous datasets,
                // not worth complicating the constructor with an extra argument.
                return std::min(length, static_cast<hsize_t>(10000));
            }
        }()),
        my_mspace(1, &my_block_size),
        my_fspace(1, &my_full_length),
        my_buffer(my_block_size)
    {}

public:
    /**
     * Load the contents of the next chunk in the dataset.
     * On return, the loaded chunk is now the "current" chunk.
     *
     * @return Number of elements loaded in the current chunk.
     * If zero is returned, the dataset traversal is complete.
     */
    hsize_t load() {
        my_last_loaded += my_available;
        my_available = std::min(my_full_length - my_last_loaded, my_block_size);
        if (my_available == 0) {
            return 0;
        }

        constexpr hsize_t zero = 0;
        my_mspace.selectHyperslab(H5S_SELECT_SET, &my_available, &zero);
        my_fspace.selectHyperslab(H5S_SELECT_SET, &my_available, &my_last_loaded);
        my_data.read(my_buffer.data(), as_numeric_datatype<Type_>(), my_mspace, my_fspace);
        return my_available;
    }

    /**
     * Get the contents of the current chunk.
     * This should only be called after `load()`.
     *
     * @return Pointer to an array containing the contents of the current chunk.
     * Only the first `X` elements should be accessed, where `X` is the return value of the most recent call to `load()`. 
     * Callers can freely modify the contents of this array.
     */
    Type_* contents() {
        return my_buffer.data();                
    }

    /**
     * Get the start position of the current chunk, i.e., the index of the first element in the chunk. 
     * This should only be called after `load()`.
     *
     * @return Start position of the current chunk.
     */
    hsize_t start() const {
        return my_last_loaded;
    }

private:
    const H5::DataSet& my_data;
    hsize_t my_full_length, my_block_size;
    H5::DataSpace my_mspace;
    H5::DataSpace my_fspace;
    std::vector<Type_> my_buffer;
    hsize_t my_last_loaded = 0;
    hsize_t my_available = 0;
};

}

}

#endif
