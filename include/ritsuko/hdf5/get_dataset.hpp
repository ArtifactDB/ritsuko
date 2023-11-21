#ifndef RITSUKO_HDF5_GET_DATASET_HPP
#define RITSUKO_HDF5_GET_DATASET_HPP

#include "H5Cpp.h"
#include <string>
#include "open.hpp"

/**
 * @file get_dataset.hpp
 * @brief Back-compatibility functions for `open.hpp`.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @cond
 */
inline H5::DataSet get_dataset(const H5::Group& handle, const char* name) {
    return open_dataset(handle, name);
}

inline H5::DataSet get_scalar_dataset(const H5::Group& handle, const char* name) {
    return open_scalar_dataset(handle, name);
}
/**
 * @endcond
 */

}

}

#endif


