#ifndef RITSUKO_IS_UTF8_STRING_HPP
#define RITSUKO_IS_UTF8_STRING_HPP

#include <type_traits>
#include <cstdint>
#include "H5Cpp.h"

/**
 * @file is_utf8_cstring.hpp
 * @brief Check if a string datatype uses null termination and UTF-8 encoding.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * Check if a HDF5 string datatype is null-terminated (i.e., C-style strings) and compatible with the UTF-8 encoding.
 *
 * This returns `false` if a Fortran-style string is used, i.e., padded with spaces.
 *
 * This returns `true` even if the string datatype uses ASCII encoding, given that ASCII is a subset of UTF-8.
 * As a result, this function is mostly performative as all valid HDF5 strings are encoded in either ASCII or UTF-8;
 * nonetheless, we run these checks to be explicit and to protect against the future addition of more encodings.
 *
 * @param stype HDF5 string datatype.
 * @return Whether `stype` uses C-style strings (i.e., null terminated) and uses a UTF-8-compatible encoding.
 */
inline bool is_utf8_cstring(const H5::StrType& stype) {
    const auto spad = stype.getStrpad();
    if (spad == H5T_STR_NULLTERM || spad == H5T_STR_NULLPAD) {
        const auto cset = stype.getCset();
        return (cset == H5T_CSET_ASCII || cset == H5T_CSET_UTF8);
    } else {
        return false;
    }
}

/**
 * Overload of `is_utf8_cstring()` that accepts a HDF5 dataset.
 * @param data A HDF5 dataset.
 * Its datatype may be of any class.
 * @return Whether the dataset contains C-style strings that use a UTF-8-compatible encoding.
 * This returns false if its datatype's class is not string.
 */
inline bool is_utf8_cstring(const H5::DataSet& data) {
    if (data.getTypeClass() != H5T_STRING) {
        return false;
    }
    return is_utf8_cstring(H5::StrType(data));
}

/**
 * Overload of `is_utf8_string()` that accepts a HDF5 attribute.
 * @param attr A HDF5 attribute.
 * Its datatype may be of any class.
 * @return Whether the attribute contains C-style strings that use a UTF-8-compatible encoding.
 * This returns false if its datatype's class is not string.
 */
inline bool is_utf8_cstring(const H5::Attribute& attr) {
    if (attr.getTypeClass() != H5T_STRING) {
        return false;
    }
    return is_utf8_cstring(attr.getStrType());
}

/**
 * @cond
 */
// For back-compatiblity.
inline bool is_utf8_string(const H5::StrType& stype) {
    return is_utf8_cstring(stype);
}

inline bool is_utf8_string(const H5::DataSet& data) {
    return is_utf8_cstring(data);
}

inline bool is_utf8_string(const H5::Attribute& attr) {
    return is_utf8_cstring(attr);
}
/**
 * @endcond
 */

}

}

#endif
