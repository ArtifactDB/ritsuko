#ifndef RITSUKO_HDF5_VLS_OPEN_HPP
#define RITSUKO_HDF5_VLS_OPEN_HPP

#include "H5Cpp.h"

#include <string>
#include <stdexcept>

#include "../open.hpp"
#include "../get_name.hpp"
#include "Pointer.hpp"

/**
 * @file open.hpp
 * @brief Open HDF5 datasets for a VLS array.
 */

namespace ritsuko {

namespace hdf5 {

namespace vls {

/**
 * Open a HDF5 dataset containing pointers into the VLS heap, used to define the individual strings in the VLS array.
 * An error is raised if the dataset's datatype is not compound or does not meet the expectations defined by `validate_pointer_datatype()`.
 *
 * @param handle Group containing the dataset of pointers.
 * @param name Name of the dataset of pointers.
 * @param offset_precision Maximum number of bits in the integer type used for the start offset, see `Pointer::offset`.
 * @param length_precision Maximum number of bits in the integer type used for the string length, see `Pointer::length`.
 *
 * @return An open handle into a HDF5 dataset.
 */
inline H5::DataSet open_pointers(const H5::Group& handle, const char* name, size_t offset_precision, size_t length_precision) {
    auto dhandle = open_dataset(handle, name);
    if (dhandle.getTypeClass() != H5T_COMPOUND) {
        throw std::runtime_error("expected a compound datatype for a VLS pointer dataset at '" + get_name(dhandle) + "'");
    }
    try {
        validate_pointer_datatype(dhandle.getCompType(), offset_precision, length_precision);
    } catch (std::exception& e) {
        throw std::runtime_error("incorrect type for VLS pointer dataset at '" + get_name(dhandle) + "; " + std::string(e.what()));
    }
    return dhandle;
}

/**
 * Open a HDF5 dataset containing the VLS heap.
 * This is expected to be a 1-dimensional dataset of unsigned 8-bit integers,
 * representing the concatenation of bytes from all the variable length strings in the VLS array.
 *
 * @param handle Group containing the dataset of pointers.
 * @param name Name of the dataset of pointers.
 *
 * @return An open handle into a HDF5 dataset.
 */
inline H5::DataSet open_heap(const H5::Group& handle, const char* name) {
    auto dhandle = open_dataset(handle, name);
    if (dhandle.getTypeClass() != H5T_INTEGER) {
        throw std::runtime_error("expected an integer datatype for the VLS heap at '" + get_name(dhandle) + "'");
    }
    if (exceeds_integer_limit(dhandle.getIntType(), 8, false)) {
        throw std::runtime_error("expected 8-bit unsigned integers for the VLS heap at '" + get_name(dhandle) + "'");
    }
    if (dhandle.getSpace().getSimpleExtentNdims() != 1) {
        throw std::runtime_error("expected a 1-dimensional dataset for the VLS heap at '" + get_name(dhandle) + "'");
    }
    return dhandle;
}

}

}

}

#endif
