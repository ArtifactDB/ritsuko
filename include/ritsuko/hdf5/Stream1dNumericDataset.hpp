#ifndef RITSUKO_HDF5_STREAM_1D_NUMERIC_DATASET_HPP
#define RITSUKO_HDF5_STREAM_1D_NUMERIC_DATASET_HPP

#include <vector>
#include <stdexcept>
#include <cassert>

#include "H5Cpp.h"
#include "sanisizer/sanisizer.hpp"

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
 * @tparam Type_ Numeric type to represent the data in memory.
 * @tparam DataSetPointer_ Class of a pointer to a `H5::DataSet`.
 * This can be raw or smart depending on the caller's management of its lifetime.
 *
 * This streams in a 1-dimensional HDF5 numeric dataset in a chunk-wise manner.
 * The aim is to enable inspection of the dataset contents while minimizing memory usage.
 */
template<typename Type_, class DataSetPointer_ = const H5::DataSet*>
class Stream1dNumericDataset {
public:
    /**
     * @param data_ptr Pointer to a HDF5 dataset.
     * It is assumed that this dataset is 1-dimensional.
     * It is also assumed that its datatype is an integer or float. 
     *
     * If `data_ptr` is a raw pointer, it should not be deleted before the last call to any methods of this `Stream1dNumericDataset` instance. 
     * @param length Length of the dataset, i.e., the extent of its sole dimension. 
     */
    Stream1dNumericDataset(DataSetPointer_ data_ptr, hsize_t length) : 
        my_data_ptr(std::move(data_ptr)), 
        my_full_length(length), 
        my_block_size([&]{
            assert(my_data_ptr->getSpace().getSimpleExtentNdims() == 1);
            hsize_t output;
            const auto& plist = my_data_ptr->getCreatePlist();
            if (plist.getLayout() == H5D_CHUNKED) {
                plist.getChunk(1, &output);
            } else {
                // Hard-coding the mock chunk size for contiguous datasets,
                // not worth complicating the constructor with an extra argument.
                output = sanisizer::min(length, 10000);
            }
            return output;
        }()),
        my_mspace(1, &my_block_size),
        my_fspace(1, &my_full_length)
    {
        assert([&]{
            const auto cls = my_data_ptr->getDataType().getClass();
            return cls == H5T_INTEGER || cls == H5T_FLOAT;
        }());
    }

public:
    /**
     * @return Size of each chunk, in terms of the number of elements.
     */
    hsize_t chunk_size() const {
        return my_block_size;
    }

    /**
     * Load the contents of the next chunk in the dataset.
     *
     * @param[out] buffer Pointer to an array of `chunk_size()`.
     * On output, this contains the contents of the current chunk in its first \f$X\f$ elements,
     * where \f$X\f$ is the return value of this method.
     *
     * @return Number of elements loaded in the current chunk.
     * If zero is returned, the dataset traversal is complete.
     */
    hsize_t load(Type_* buffer) {
        my_last_loaded += my_available;
        my_available = std::min(my_full_length - my_last_loaded, my_block_size);
        if (my_available == 0) {
            return 0;
        }

        constexpr hsize_t zero = 0;
        my_mspace.selectHyperslab(H5S_SELECT_SET, &my_available, &zero);
        my_fspace.selectHyperslab(H5S_SELECT_SET, &my_available, &my_last_loaded);
        my_data_ptr->read(buffer, as_numeric_datatype<Type_>(), my_mspace, my_fspace);
        return my_available;
    }

    /**
     * Get the start position of the current chunk, i.e., the index of the first element in the chunk. 
     * That is, `buffer[j]` corresponds to the `start() + j`-th element of the dataset.
     * This should only be called after `load()`.
     *
     * @return Start position of the current chunk.
     */
    hsize_t start() const {
        return my_last_loaded;
    }

private:
    DataSetPointer_ my_data_ptr;
    hsize_t my_full_length, my_block_size;
    H5::DataSpace my_mspace;
    H5::DataSpace my_fspace;
    hsize_t my_last_loaded = 0;
    hsize_t my_available = 0;
};

}

}

#endif
