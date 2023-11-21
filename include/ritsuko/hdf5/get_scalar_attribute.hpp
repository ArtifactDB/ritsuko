#ifndef RITSUKO_HDF5_GET_ATTRIBUTE_HPP
#define RITSUKO_HDF5_GET_ATTRIBUTE_HPP

#include "H5Cpp.h"
#include <string>
#include "open.hpp"

/**
 * @file get_scalar_attribute.hpp
 * @brief Back-compatibility wrappers for `open.hpp`.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @cond
 */
template<class Object_>
H5::Attribute get_scalar_attribute(const Object_& handle, const char* name) {
    return open_scalar_attribute(handle, name);
}
/**
 * @endcond
 */

}

}

#endif
