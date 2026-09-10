#ifndef RITSUKO_HDF5_STREAM_1D_STRING_DATASET_HPP
#define RITSUKO_HDF5_STREAM_1D_STRING_DATASET_HPP

#include "H5Cpp.h"

#include <vector>
#include <string>
#include <stdexcept>

#include "get_name.hpp"
#include "strnlen.hpp"
#include "ReclaimVlsMemory.hpp"

/**
 * @file Stream1dStringDataset.hpp
 * @brief Stream a numeric 1-dimensional HDF5 dataset into memory.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @brief Stream a 1-dimensional HDF5 string dataset into memory.
 * @tparam DataSetPointer_ Class of a pointer to a `H5::DataSet`.
 * This can be raw or smart depending on the caller's management of its lifetime.
 *
 * This streams in a 1-dimensional HDF5 string dataset in a chunk-wise manner.
 * The aim is to enable inspection of the dataset contents while minimizing memory usage.
 */
template<class DataSetPointer_ = const H5::DataSet*>
class Stream1dStringDataset {
public:
    /**
     * @param data_ptr Pointer to a HDF5 dataset. 
     * It is assumed that this dataset is 1-dimensional.
     * It is also assumed that its datatype is an integer or float. 
     *
     * If `data_ptr` is a raw pointer, it should not be deleted before the last call to any methods of this `Stream1dStringDataset` instance. 
     * @param length Length of the dataset, i.e., the extent of its sole dimension.
     */
    Stream1dStringDataset(DataSetPointer_ data_ptr, hsize_t length) :
        my_data_ptr(std::move(data_ptr)), 
        my_full_length(length), 
        my_block_size([&]{
            const auto& plist = my_data_ptr->getCreatePlist();
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
        my_dtype(my_data_ptr->getDataType()),
        my_is_variable(my_dtype.isVariableStr())
    {
        if (my_is_variable) {
            my_var_buffer.resize(my_block_size);
        } else {
            my_fixed_length = my_dtype.getSize();
            my_fix_buffer.resize(my_fixed_length * my_block_size);
        }
        my_final_buffer.resize(my_block_size);
    }

public:
    /**
     * Load the contents of the next chunk in the dataset.
     * On return, this is now the "current" chunk.
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

        if (my_is_variable) {
            my_data_ptr->read(my_var_buffer.data(), my_dtype, my_mspace, my_fspace);
            const auto& plist = H5::DSetMemXferPropList::DEFAULT;
            [[maybe_unused]] ReclaimVlsMemory deletor(my_dtype.getId(), my_mspace.getId(), plist.getId(), my_var_buffer.data());
            for (hsize_t i = 0; i < my_available; ++i) {
                if (my_var_buffer[i] == NULL) {
                    throw std::runtime_error("detected a NULL pointer for a variable length string in '" + get_name(*my_data_ptr) + "'");
                }
                auto& curstr = my_final_buffer[i];
                curstr.clear();
                curstr.insert(0, my_var_buffer[i]);
            }

        } else {
            auto bptr = my_fix_buffer.data();
            my_data_ptr->read(bptr, my_dtype, my_mspace, my_fspace);
            for (size_t i = 0; i < my_available; ++i, bptr += my_fixed_length) {
                auto& curstr = my_final_buffer[i];
                curstr.clear();
                curstr.insert(curstr.end(), bptr, bptr + strnlen(bptr, my_fixed_length));
            }
        }

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
    std::string* contents() {
        return my_final_buffer.data();                
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
    DataSetPointer_ my_data_ptr;
    hsize_t my_full_length, my_block_size;
    H5::DataSpace my_mspace;
    H5::DataSpace my_fspace;

    H5::DataType my_dtype;
    bool my_is_variable;
    std::vector<char*> my_var_buffer;
    std::size_t my_fixed_length = 0;
    std::vector<char> my_fix_buffer;
    std::vector<std::string> my_final_buffer;

    hsize_t my_last_loaded = 0;
    hsize_t my_available = 0;
};

}

}

#endif
