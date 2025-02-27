#ifndef RITSUKO_HDF5_VLS_HPP
#define RITSUKO_HDF5_VLS_HPP

#include "open.hpp"
#include "Pointer.hpp"
#include "Stream1dArray.hpp"

/**
 * @file vls.hpp
 * @brief Utilities for handling **ritsuko**'s custom VLS arrays.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @namespace ritsuko::hdf5::vls
 * @brief Assorted functions for handling **ritsuko**'s custom VLS arrays.
 *
 * One weakness of HDF5 is its inability to efficiently handle variable length string (VLS) arrays.
 * Storing them as fixed-length strings requires padding all strings to the longest string,
 * causing an inflation in disk usage that cannot be completely countered by compression.
 * On the other hand, HDF5's own VLS datatype does not compress the strings themselves, only the pointers to those strings.
 *
 * To patch over this weakness in current versions of HDF5, **ritsuko** introduces its own concept of a VLS array.
 * This is defined by two HDF5 datasets - one storing a VLS heap, and another storing pointers into that heap.
 *
 * - The VLS heap is a 1-dimensional dataset of unsigned 8-bit integers, containing the concatenation of all variable length strings in the array.
 *   This is decoded to a string by casting the integers to `unsigned char` and then reading them via an aliased `char *`.
 * - The pointer dataset is an \f$N\f$-dimensional dataset of a compound datatype defined by `define_pointer_datatype()`.
 *   Each entry of the pointer dataset contains the starting offset and length of a single string on the heap. 
 *
 * The idea is to read the pointer dataset into an array of `Pointer` instances,
 * and then use the offset and length for each `Pointer` to extract a slice of characters from the heap.
 * Each slice defines a string, possibly of length shorter than the slice if an intervening null terminator is present.
 * Each slice should lie within the range of the heap; otherwise, there are no restrictions.
 * Slices for consecutive `Pointer`s do not have to be ordered or contiguous,
 * and different `Pointer`s can even refer to the same or overlapping slices (e.g., to improve compression for repeated strings).
 * 
 * Typically the pointer and heap datasets for a single VLS array will be stored in their own group,
 * where they can be opened by `open_pointers()` and `open_heap()`, respectively.
 * See also `Stream1dArray` to quickly stream the contents of a VLS array.
 */
namespace vls {}

}

}

#endif
