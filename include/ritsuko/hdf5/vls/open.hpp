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
 * Open a HDF5 dataset containing pointers into the VLS heap, used to define the individual strings in a VLS array.
 *
 * There are no restrictions on the ordering of entries in the pointer dataset.
 * Slices for consecutive `Pointer`s do not have to be ordered or contiguous.
 * This allows one or more entries in the `Pointer` dataset to be modified without invalidating other entries.
 * Different `Pointer`s can even refer to the same or even overlapping slices, which provides some opportunities to improve compression for repeated strings.
 *
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
 * This should be a 1-dimensional dataset of unsigned 8-bit integers, representing the concatenation of bytes from all the variable length strings in the VLS array.
 * Ideally, all bytes are referenced by at least one `Pointer` in the associated pointer dataset, though this is not required, e.g., if a VLS is replaced with a shorter string.
 *
 * We use an integer datatype rather than HDF5's own string datatypes to avoid the risk of a naive incorrect interpretation of the heap as an array of fixed-width strings.
 *
 * When reading the heap into memory, users are advised to first load the bytes into an `unsigned char` array and then read them via an aliased `char *`.
 * This preserves the bit patterns by avoiding a HDF5-mediated conversion between the dataset's unsigned 8-bit integer datatype and a possibly-signed `char` type.
 * Conversely, when creating a heap dataset, we should use an aliased `unsigned char *` to access the contents C-style strings.
 * We write this to the heap dataset by using HDF5 to trivially convert from the `NATIVE_UCHAR` memory type to the dataset's 8-bit unsigned integer datatype.
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
