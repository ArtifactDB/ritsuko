#ifndef RITSUKO_HDF5_MISSING_PLACEHOLDER_HPP
#define RITSUKO_HDF5_MISSING_PLACEHOLDER_HPP

#include "H5Cpp.h"
#include <string>
#include <optional>

#include "as_hdf5_numeric_type.hpp"
#include "load_hdf5_attribute.hpp"
#include "validate_hdf5_string.hpp"
#include "get_hdf5_1d_length.hpp"
#include "get_hdf5_name.hpp"

/**
 * @file missing_placeholder.hpp
 * @brief Get the missing placeholder attribute.
 */

namespace ritsuko {

/**
 * Check the validity of a missing placeholder attribute on a numeric dataset.
 * An error is raised if the attribute is not a scalar or has a different type than the dataset.
 *
 * @param dset Dataset handle.
 * @param attr Handle for the attribute containing the missing placeholder, typically attached to `dset`.
 */
inline void check_hdf5_numeric_missing_placeholder_attribute(const H5::DataSet& dset, const H5::Attribute& attr) {
    if (!is_hdf5_scalar(attr)) {
        throw std::runtime_error("expected the '" + get_name(attr) + "' attribute to be a scalar");
    }
    if (attr.getDataType() != dset.getDataType()) {
        throw std::runtime_error("expected the '" + get_name(attr) + "' attribute to have the same type as its dataset");
    }
}

/**
 * Check if a missing placeholder attribute is present, and if so, open it and loads it value.
 * This will also call `check_hdf5_numeric_missing_placeholder_attribute()` to validate the placeholder's properties.
 *
 * @tparam Type_ Type to use to store the data in memory, see `as_numeric_datatype()` for supported types.
 * @param handle Dataset handle.
 * @param attr_name Name of the attribute containing the missing value placeholder.
 *
 * @return Optional value of the placeholder.
 */
template<typename Type_>
std::optional<Type_> open_and_load_optional_hdf5_numeric_missing_placeholder(const H5::DataSet& handle, const char* attr_name) {
    if (!handle.attrExists(attr_name)) {
        return std::optional<Type_>();
    }
    auto ahandle = handle.openAttribute(attr_name);
    check_hdf5_numeric_missing_placeholder_attribute(handle, ahandle);
    Type_ value;
    ahandle.read(as_numeric_datatype<Type_>(), &value);
    return value;
}

/**
 * @cond
 */
inline void check_hdf5_string_missing_placeholder_attribute_preliminary(const H5::Attribute& attr) {
    if (!is_scalar(attr)) {
        throw std::runtime_error("expected the '" + get_name(attr) + "' attribute to be a scalar");
    }
    if (attr.getTypeClass() != H5T_STRING) {
        throw std::runtime_error("expected the '" + get_name(attr) + "' attribute to have a string datatype");
    }
}
/**
 * @endcond
 */

/**
 * Check the validity of a missing placeholder attribute on a string dataset.
 * An error is raised if the attribute is not a scalar or has a different type class.
 * For variable length string attributes, this function will also check that the string is not NULL.
 *
 * @param attr Handle for the attribute containing the missing placeholder.
 */
inline void check_hdf5_string_missing_placeholder_attribute(const H5::Attribute& attr) {
    internal::check_hdf5_string_missing_placeholder_attribute_preliminary(attr);
    validate_scalar_string_attribute(attr);
}

/**
 * Check if a missing string placeholder attribute is present, and if so, open it and loads it value.
 * This will also call `check_hdf5_string_missing_placeholder_attribute()` to validate the placeholder's properties.
 *
 * @param handle Dataset handle.
 * @param attr_name Name of the attribute containing the missing value placeholder.
 * 
 * @return Optional value of the placeholder.
 */
inline std::optional<std::string> open_and_load_optional_hdf5_string_missing_placeholder(const H5::DataSet& handle, const char* attr_name) {
    if (!handle.attrExists(attr_name)) {
        return {};
    }
    auto ahandle = handle.openAttribute(attr_name);
    internal::check_hdf5_string_missing_placeholder_attribute_preliminary(ahandle);
    return load_scalar_string_attribute(ahandle);
}

}

}

#endif
