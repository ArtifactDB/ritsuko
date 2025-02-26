#ifndef RITSUKO_HDF5_VLS_OPEN_HPP
#define RITSUKO_HDF5_VLS_OPEN_HPP

#include "H5Cpp.h"

#include <string>
#include <stdexcept>

#include "../open.hpp"
#include "../get_name.hpp"
#include "define_pointer_datatype.hpp"

namespace ritsuko {

namespace hdf5 {

namespace vls {

inline H5::DataSet open_pointers(const H5::Group& handle, const char* name, size_t start_precision, size_t size_precision) {
    auto dhandle = open_dataset(handle, name);
    if (dhandle.getTypeClass() != H5T_COMPOUND) {
        throw std::runtime_error("expected a compound datatype for a VLS pointer dataset at '" + get_name(dhandle) + "'");
    }
    try {
        validate_pointer_datatype(dhandle.getCompType(), start_precision, size_precision);
    } catch (std::exception& e) {
        throw std::runtime_error("incorrect type for VLS pointer dataset at '" + get_name(dhandle) + "; " + std::string(e.what()));
    }
    return dhandle;
}

inline H5::DataSet open_concatenated(const H5::Group& handle, const char* name) {
    auto dhandle = open_dataset(handle, name);
    if (dhandle.getTypeClass() != H5T_INTEGER) {
        throw std::runtime_error("expected an integer datatype for a VLS concatenated dataset at '" + get_name(dhandle) + "'");
    }
    if (exceeds_integer_limit(dhandle.getIntType(), 8, false)) {
        throw std::runtime_error("concatenated string buffer for VLS data at '" + get_name(dhandle) + "' should be stored as 8-bit unsigned integers");
    }
    return dhandle;
}

}

}

}

#endif
