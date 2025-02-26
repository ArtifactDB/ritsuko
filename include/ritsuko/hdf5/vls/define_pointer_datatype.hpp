#ifndef RITSUKO_HDF5_VLS_define_pointer_datatype_HPP
#define RITSUKO_HDF5_VLS_define_pointer_datatype_HPP

#include "H5Cpp.h"

#include <stdexcept>
#include <string>

#include "../as_numeric_datatype.hpp"
#include "../exceeds_limit.hpp"

namespace ritsuko {

namespace hdf5 {

namespace vls {

template<typename Start_, typename Size_>
struct Pointer {
    Start_ start;
    Size_ size;
};

template<typename Start_, typename Size_>
H5::CompType define_pointer_datatype() {
    typedef Pointer<Start_, Size_> tmp;
    H5::CompType pointer_type(sizeof(tmp));
    pointer_type.insertMember("start", HOFFSET(tmp, start), as_numeric_datatype<Start_>());
    pointer_type.insertMember("size", HOFFSET(tmp, size), as_numeric_datatype<Size_>());
    return pointer_type;
}

inline void validate_pointer_datatype(const H5::CompType& type, size_t start_precision, size_t size_precision) {
    if (type.getNmembers() != 2) {
        throw std::runtime_error("expected VLS compound datatype to have two members");
    }

    if (type.getMemberName(0) != "start") {
        throw std::runtime_error("first member of a VLS compound datatype should be named 'start'");
    }
    if (type.getMemberClass(0) != H5T_INTEGER) {
        throw std::runtime_error("first member of a VLS compound datatype should have integer type");
    }
    auto start_type = type.getMemberIntType(0);
    if (exceeds_integer_limit(start_type, start_precision, false)) {
        throw std::runtime_error("first member of a VLS compound datatype should not exceed a " + std::to_string(start_precision) + "-bit unsigned integer");
    }

    if (type.getMemberName(1) != "size") {
        throw std::runtime_error("second member of a VLS compound datatype should be named 'size'");
    }
    if (type.getMemberClass(0) != H5T_INTEGER) {
        throw std::runtime_error("second member of a VLS compound datatype should have integer type");
    }
    auto size_type = type.getMemberIntType(1);
    if (exceeds_integer_limit(size_type, size_precision, false)) {
        throw std::runtime_error("second member of a VLS compound datatype should not exceed a " + std::to_string(size_precision) + "-bit unsigned integer");
    }
}

}

}

}

#endif 
