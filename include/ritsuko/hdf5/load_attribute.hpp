#ifndef RITSUKO_HDF5_LOAD_SCALAR_STRING_ATTRIBUTE_HPP
#define RITSUKO_HDF5_LOAD_SCALAR_STRING_ATTRIBUTE_HPP

#include "H5Cpp.h"
#include <string>

#include "get_1d_length.hpp"
#include "as_numeric_datatype.hpp"

/**
 * @file load_scalar_string_attribute.hpp
 * @brief Load a scalar string HDF5 attribute.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @param attr Handle to a scalar string attribute.
 * @return The attribute as a string.
 */
inline std::string load_scalar_string_attribute(const H5::Attribute& attr) {
    std::string output;
    attr.read(attr.getStrType(), output);
    return output;
}

/**
 * @tparam check_ Whether to check that `attr` is a 1-dimensional string attribute.
 * @param attr Handle to a 1-dimensional string attribute.
 * @param full_length Length of the attribute in `attr`, usually obtained by `get_1d_length()`.
 * @return Vector of strings.
 */
inline std::vector<std::string> load_1d_string_attribute(const H5::Attribute& attr, hsize_t full_length) {
    auto dtype = attr.getDataType();
    auto mspace = attr.getSpace();
    std::vector<std::string> output;
    output.reserve(full_length);

    if (dtype.isVariableStr()) {
        std::vector<char*> buffer(full_length);
        try {
            attr.read(dtype, buffer.data());
            for (hsize_t i = 0; i < full_length; ++i) {
                output.emplace_back(buffer[i]);
            }
        } catch (...) {
            H5Dvlen_reclaim(dtype.getId(), mspace.getId(), H5P_DEFAULT, buffer.data());
            throw;
        }

    } else {
        size_t len = dtype.getSize();
        std::vector<char> buffer(len * full_length);
        attr.read(dtype, buffer.data());
        auto ptr = buffer.data();
        for (size_t i = 0; i < full_length; ++i, ptr += len) {
            size_t j = 0;
            for (; j < len && ptr[j] != '\0'; ++j) {}
            output.emplace_back(ptr, ptr + j);
        }
    }

    return output;
}

/**
 * Overload of `load_1d_string_attribute()` that determines the length of the attribute via `get_1d_length()`.
 * @param attr Handle to a 1-dimensional string attribute.
 * @return Vector of strings.
 */
inline std::vector<std::string> load_1d_string_attribute(const H5::Attribute& attr) {
    return load_1d_string_attribute(attr, get_1d_length(attr.getSpace(), false));
}

/**
 * @tparam Type_ Type for holding the data in memory, see `as_numeric_datatype()` for supported types.
 * @param attr Handle to a scalar numeric attribute.
 * @return The value of the attribute.
 *
 * Unlike `load_scalar_string_attribute()`, no error is raised if `attr` is not an appropriate type for `Type_`.
 * Callers are responsible for checking that the right type is present, e.g., with `exceeds_integer_limit()`.
 */
template<typename Type_>
Type_ load_scalar_numeric_attribute(const H5::Attribute& attr) {
    Type_ val;
    auto mtype = as_numeric_datatype<Type_>();
    attr.read(mtype, &val);
    return val;
}

/**
 * @tparam Type_ Type for holding the data in memory, see `as_numeric_datatype()` for supported types.
 * @param attr Handle to a numeric attribute.
 * @param full_length Length of the attribute in `attr`, usually obtained by `get_1d_length()`.
 * @return Vector containing the contents of the attribute.
 * 
 * Unlike `load_1d_string_attribute()`, no error is raised if `attr` is not an appropriate type for `Type_`.
 * Callers are responsible for checking that the right type is present, e.g., with `exceeds_integer_limit()`.
 */
template<typename Type_>
std::vector<Type_> load_1d_numeric_attribute(const H5::Attribute& attr, hsize_t full_length) {
    auto mtype = as_numeric_datatype<Type_>();
    std::vector<Type_> buffer(full_length);
    attr.read(mtype, buffer.data());
    return buffer;
}

/**
 * Overload of `load_1d_numeric_attribute()` that determines the length of the attribute via `get_1d_length()`.
 * @tparam Type_ Type for holding the data in memory, see `as_numeric_datatype()` for supported types.
 * @param attr Handle to a numeric attribute.
 * @return Vector containing the contents of the attribute.
 *
 * Unlike `load_1d_string_attribute()`, no error is raised if `attr` is not an appropriate type for `Type_`.
 * Callers are responsible for checking that the right type is present, e.g., with `exceeds_integer_limit()`.
 */
template<typename Type_>
std::vector<Type_> load_1d_numeric_attribute(const H5::Attribute& attr) {
    return load_1d_numeric_attribute<Type_>(attr, get_1d_length(attr.getSpace(), false));
}

}

}

#endif
