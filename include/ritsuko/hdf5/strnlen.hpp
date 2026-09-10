#ifndef RITSUKO_HDF5_UTILS_STRINGS_HPP
#define RITSUKO_HDF5_UTILS_STRINGS_HPP

#include "H5Cpp.h"

#include <cstddef>

/**
 * @file strnlen.hpp
 * @brief Determine the length of a fixed-size string.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * Get the length of a fixed-size string, either by searching for the first null terminator or by reaching the `max` length.
 *
 * @param ptr Pointer to an array of characters, possibly containing a C-style string.
 * @param max Maximum length of the array referenced by `ptr`.
 *
 * @return The number of characters to the first occurence of the null terminator or `max`, depending on which is smaller.
 */
inline std::size_t strnlen(const char* ptr, std::size_t max) {
    std::size_t j = 0;
    for (; j < max && ptr[j] != '\0'; ++j) {}
    return j;
}

}

}

#endif
