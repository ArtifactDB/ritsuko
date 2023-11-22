#ifndef RITSUKO_HDF5_LOAD_SCALAR_DATASET_HPP
#define RITSUKO_HDF5_LOAD_SCALAR_DATASET_HPP

#include <string>
#include <vector>

#include "H5Cpp.h"

#include "get_name.hpp"
#include "_strings.hpp"

/**
 * @file load_scalar_dataset.hpp
 * @brief Helper functions to load a scalar dataset.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * Load a scalar string dataset into a single string.
 * @param handle Handle to the HDF5 scalar dataset.
 * @return String containing the contents of the sole dataset entry.
 */
inline std::string load_scalar_string_dataset(const H5::DataSet& handle) {
    auto dtype = handle.getDataType();
    if (dtype.isVariableStr()) {
        char* vptr;
        handle.read(&vptr, dtype);
        [[maybe_unused]] VariableStringCleaner deletor(dtype.getId(), handle.getSpace().getId(), &vptr);
        if (vptr == NULL) {
            throw std::runtime_error("detected a NULL pointer for a variable length string in '" + get_name(handle) + "'");
        }
        std::string output(vptr);
        return output;
    } else {
        size_t fixed_length = dtype.getSize();
        std::vector<char> buffer(fixed_length);
        handle.read(buffer.data(), dtype);
        return std::string(buffer.begin(), buffer.begin() + find_string_length(buffer.data(), fixed_length));
    }
}

inline void validate_1d_string_dataset(const H5::DataSet& handle, hsize_t full_length, hsize_t buffer_size) {
    auto dtype = handle.getDataType();
    if (!dtype.isVariableStr()) {
        return;
    }

    hsize_t block_size = pick_1d_block_size(ptr->getCreatePlist(), full_length, buffer_size);
    for (
        ptr->read(var_buffer.data(), dtype, mspace, dspace);
        [[maybe_unused]] VariableStringCleaner deletor(dtype.getId(), mspace.getId(), var_buffer.data());
        available = std::min(full_length - last_loaded, block_size);

        for (hsize_t i = 0; i < full_length; ++i) {
            
        }
    }
}

}

}

#endif
