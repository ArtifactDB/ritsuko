#ifndef RITSUKO_HDF5_GET_MISSING_PLACEHOLDER_ATTRIBUTE_HPP
#define RITSUKO_HDF5_GET_MISSING_PLACEHOLDER_ATTRIBUTE_HPP

#include "H5Cpp.h"
#include <string>

#include "as_numeric_datatype.hpp"
#include "load_attribute.hpp"

/**
 * @file missing_placeholder.hpp
 * @brief Get the missing placeholder attribute.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * Open a handle to a missing placeholder's attribute.
 * It is assumed that the caller has already checked `handle` for the existence of the attribute at `attr_name`;
 * this function will only perform additional checks for the shape and type of the attribute.
 *
 * @param handle Dataset handle.
 * @param attr_name Name of the attribute containing the missing value placeholder.
 * @param type_class_only Whether to only require identical type classes for the placeholder.
 * By default, we require identity in the types themselves.
 * 
 * @return Handle to the attribute.
 * An error is raised if the attribute is not a scalar or has a different type (or type class, if `type_class_only_ = true`) to the dataset.
 */
inline H5::Attribute open_missing_placeholder_attribute(const H5::DataSet& handle, const char* attr_name, bool type_class_only = false) {
    auto attr = handle.openAttribute(attr_name);
    if (attr.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error("expected the '" + std::string(attr_name) + "' attribute to be a scalar");
    }

    if (type_class_only) {
        if (attr.getTypeClass() != handle.getTypeClass()) {
            throw std::runtime_error("expected the '" + std::string(attr_name) + "' attribute to have the same type class as its dataset");
        }
    } else {
        if (attr.getDataType() != handle.getDataType()) {
            throw std::runtime_error("expected the '" + std::string(attr_name) + "' attribute to have the same type as its dataset");
        }
    }

    return attr;
}

/**
 * @cond
 */
// Back-compatibility only.
inline H5::Attribute get_missing_placeholder_attribute(const H5::DataSet& handle, const char* attr_name, bool type_class_only = false) {
    return open_missing_placeholder_attribute(handle, attr_name, type_class_only);
}
/**
 * @cond
 */

/**
 * Load a missing numeric placeholder by calling `open_missing_placeholder_attribute()`.
 *
 * @tparam Type_ Type to use to store the data in memory, see `as_numeric_datatype()` for supported types.
 * @param handle Dataset handle.
 * @param attr_name Name of the attribute containing the missing value placeholder.
 * @return Pair containing (i) a boolean indicating whether the placeholder attribute was present, and (ii) the value of the placeholder if the first element is `true`.
 */
template<typename Type_>
std::pair<bool, Type_> load_numeric_missing_placeholder(const H5::DataSet& handle, const char* attr_name) {
    std::pair<bool, Type_> output(false, 0);
    if (!handle.attrExists(attr_name)) {
        return output;
    }
    output.first = true;
    auto ahandle = open_missing_placeholder_attribute(handle, attr_name, /* type_class_only = */ false);
    ahandle.read(as_numeric_datatype<Type_>(), &(output.second));
    return output;
}

/**
 * Load a missing string placeholder by calling `open_missing_placeholder_attribute()`.
 * Note the difference in the default for `type_class_only`.
 *
 * @param handle Dataset handle.
 * @param attr_name Name of the attribute containing the missing value placeholder.
 * @param type_class_only Whether to only require identical type classes for the placeholder.
 * This is set to `true` as all strings are ultimately treated as null-terminated byte arrays.
 *
 * @return Pair containing (i) a boolean indicating whether the placeholder attribute was present, and (ii) the value of the placeholder if the first element is `true`.
 */
inline std::pair<bool, std::string> load_string_missing_placeholder(const H5::DataSet& handle, const char* attr_name, bool type_class_only = true) {
    std::pair<bool, std::string> output(false, "");
    if (!handle.attrExists(attr_name)) {
        return output;
    }
    output.first = true;
    auto ahandle = open_missing_placeholder_attribute(handle, attr_name, type_class_only);
    output.second = load_scalar_string_attribute(ahandle);
    return output;
}

}

}

#endif
