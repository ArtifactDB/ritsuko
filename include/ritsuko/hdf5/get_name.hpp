#ifndef RITSUKO_HDF5_GET_NAME_HPP
#define RITSUKO_HDF5_GET_NAME_HPP

#include "H5Cpp.h"
#include <string>
#include <vector>

/**
 * @file get_name.hpp
 * @brief Get the name of a HDF5 object.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * Get the name of a HDF5 object, usually for printing informative error messages.
 * @tparam Object_ Type of HDF5 object, usually a `Group`, `DataSet` or `Attribute`.
 * @param obj A HDF5 object.
 * @return Name of the HDF5 object inside the file.
 */
template<class Object_>
std::string get_name(const Object_& obj) {
    if constexpr(std::is_same<Object_, H5::Attribute>::value) {
        std::string name;
        obj.getName(name);
        return name;
    } else {
        size_t len = H5Iget_name(obj.getId(), NULL, 0);
        std::vector<char> buffer(len + 1);
        H5Iget_name(obj.getId(), buffer.data(), buffer.size());
        return std::string(buffer.begin(), buffer.begin() + len);
    }
}

}

}

#endif
