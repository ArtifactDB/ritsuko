#ifndef RITSUKO_HDF5_GET_GROUP_HPP
#define RITSUKO_HDF5_GET_GROUP_HPP

#include "H5Cpp.h"
#include <string>

/**
 * @file get_group.hpp
 * @brief Quick functions to get a group handle.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @param handle Parent group (or file).
 * @param name Name of the group.
 * @return Handle to the group.
 * An error is raised if `name` does not refer to a dataset. 
 */
inline H5::Group get_group(const H5::Group& handle, const char* name) {
    if (!handle.exists(name) || handle.childObjType(name) != H5O_TYPE_GROUP) {
        throw std::runtime_error("expected a group at '" + std::string(name) + "'");
    }
    return handle.openGroup(name);
}

}

}

#endif
