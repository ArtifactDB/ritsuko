#ifndef RITSUKO_HDF5_OPEN_FILE_HPP
#define RITSUKO_HDF5_OPEN_FILE_HPP

#include "H5Cpp.h"

#include <filesystem>
#include <stdexcept>

/**
 * @file open_file.hpp
 * @brief Quick functions to open a read-only HDF5 file.
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

}

}

#endif
