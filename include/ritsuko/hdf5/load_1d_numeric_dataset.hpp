#ifndef RITSUKO_HDF5_LOAD_1D_NUMERIC_DATASET_HPP
#define RITSUKO_HDF5_LOAD_1D_NUMERIC_DATASET_HPP

#include "H5Cpp.h"
#include <vector>

#include "pick_1d_block_size.hpp"
#include "iterate_1d_blocks.hpp"
#include "as_numeric_datatype.hpp"

/**
 * @file load_1d_numeric_dataset.hpp
 * @brief Load and iterate over a 1-dimensional HDF5 numeric dataset.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * Iterate across a numeric dataset, extracting each numeric and running a user-specified function.
 *
 * @tparam Type_ Type for holding the data in memory, see `as_numeric_datatype()` for supported types.
 * @tparam Function_ Function class that accepts `(const Type_* data, size_t start, size_t len)`
 * where `data` contains all values from the interval `[start, start + len)` in the dataset.
 *
 * @param handle Handle to a numeric dataset.
 * @param full_length Length of the dataset in `handle`, usually obtained by `get_1d_length()`.
 * @param buffer_size Buffer size to use for iteration in `iterate_1d_blocks()`.
 * @param fun Function to be called on each numeric.
 * It can be assumed that the consecutive calls to `fun` will operate on contiguous intervals.
 */
template<typename Type_, class Function_>
void load_1d_numeric_dataset(const H5::DataSet& handle, hsize_t full_length, hsize_t buffer_size, Function_ fun) {
    auto block_size = pick_1d_block_size(handle.getCreatePlist(), full_length, buffer_size);
    auto dtype = handle.getDataType();

    std::vector<Type_> buffer(block_size);
    auto mtype = as_numeric_datatype<Type_>();

    iterate_1d_blocks(
        full_length, 
        block_size, 
        [&](hsize_t start, hsize_t len, const H5::DataSpace& mspace, const H5::DataSpace& dspace) -> void {
            handle.read(buffer.data(), mtype, mspace, dspace);
            fun(buffer.data(), start, len);
        }
    );
}

/**
 * Load a numeric attribute.
 *
 * @tparam Type_ Type for holding the data in memory, see `as_numeric_datatype()` for supported types.
 * @param handle Handle to a numeric attribute.
 * @param full_length Length of the attribute in `handle`, usually obtained by `get_1d_length()`.
 *
 * @return Vector containing the contents of the attribute.
 */
template<typename Type_>
std::vector<Type_> load_1d_numeric_attribute(const H5::Attribute& handle, hsize_t full_length) {
    auto mtype = as_numeric_datatype<Type_>();
    std::vector<Type_> buffer(full_length);
    handle.read(mtype, buffer.data());
    return buffer;
}

}

}

#endif

