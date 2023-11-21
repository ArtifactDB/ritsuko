#ifndef RITSUKO_HDF5_OPEN_HPP
#define RITSUKO_HDF5_OPEN_HPP

#include "H5Cpp.h"

#include <filesystem>
#include <stdexcept>
#include <string>

/**
 * @file open.hpp
 * @brief Convenience functions to safely open HDF5 handles.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @param path Path to a HDF5 file.
 * @return Handle to the file.
 * An error is raised if `path` does not exist.
 */
inline H5::H5File open_file(const std::filesystem::path& path) try {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("no file is present at '" + path.string() + "'");
    }
    return H5::H5File(path, H5F_ACC_RDONLY);
} catch (H5::Exception& e) {
    throw std::runtime_error("failed to open the HDF5 file at '" + path.string() + "'; " + e.getDetailMsg());
}

/**
 * @param handle Parent group (or file).
 * @param name Name of the group.
 * @return Handle to the group.
 * An error is raised if `name` does not refer to a dataset. 
 */
inline H5::Group open_group(const H5::Group& handle, const char* name) {
    if (!handle.exists(name) || handle.childObjType(name) != H5O_TYPE_GROUP) {
        throw std::runtime_error("expected a group at '" + std::string(name) + "'");
    }
    return handle.openGroup(name);
}

/**
 * @param handle Group containing the dataset.
 * @param name Name of the dataset inside the group.
 * @return Handle to the dataset.
 * An error is raised if `name` does not refer to a dataset. 
 */
inline H5::DataSet open_dataset(const H5::Group& handle, const char* name) {
    if (!handle.exists(name) || handle.childObjType(name) != H5O_TYPE_DATASET) {
        throw std::runtime_error("expected a dataset at '" + std::string(name) + "'");
    }
    return handle.openDataSet(name);
}

/**
 * @param handle Group containing the scalar dataset.
 * @param name Name of the dataset inside the group.
 * @return Handle to a scalar dataset.
 * An error is raised if `name` does not refer to a scalar dataset. 
 */
inline H5::DataSet open_scalar_dataset(const H5::Group& handle, const char* name) {
    auto dhandle = open_dataset(handle, name);
    auto dspace = dhandle.getSpace();
    int ndims = dspace.getSimpleExtentNdims();
    if (ndims != 0) {
        throw std::runtime_error("expected a scalar dataset at '" + std::string(name) + "'");
    }
    return dhandle;
}

/**
 * @tparam Object_ Type of the HDF5 handle, usually a `DataSet` or `Group`.
 * @param handle HDF5 dataset or group handle.
 * @param name Name of the attribute.
 *
 * @return Attribute handle.
 * An error is raised if `name` does not refer to an attribute.
 */
template<class Object_>
H5::Attribute open_attribute(const Object_& handle, const char* name) {
    if (!handle.attrExists(name)) {
        throw std::runtime_error("expected an attribute at '" + std::string(name) + "'");
    }
    return handle.openAttribute(name);
}

/**
 * @tparam Object_ Type of the HDF5 handle, usually a `DataSet` or `Group`.
 * @param handle HDF5 dataset or group handle.
 * @param name Name of the attribute.
 *
 * @return Attribute handle.
 * An error is raised if `name` does not refer to a scalar attribute.
 */
template<class Object_>
H5::Attribute open_scalar_attribute(const Object_& handle, const char* name) {
    auto attr = open_attribute(handle,name);
    if (attr.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error("expected a scalar attribute at '" + std::string(name) + "'");
    }
    return attr;
}

}

}

#endif
