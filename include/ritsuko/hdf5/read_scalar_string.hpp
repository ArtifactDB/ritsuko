#ifndef RITSUKO_HDF5_READ_SCALAR_STRING_HPP
#define RITSUKO_HDF5_READ_SCALAR_STRING_HPP

#include "H5Cpp.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <cassert>

#include "get_name.hpp"
#include "Stream1dStringDataset.hpp"
#include "Stream1dNumericDataset.hpp"
#include "as_numeric_datatype.hpp"
#include "strnlen.hpp"
#include "ReclaimVlsMemory.hpp"

/**
 * @file read_scalar_string.hpp
 * @brief Convenience functions to read scalar strings.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @param handle Handle to the HDF5 scalar dataset.
 * @return String containing the contents of the sole dataset entry.
 */
inline std::string read_scalar_string(const H5::DataSet& handle) {
    auto dtype = handle.getDataType();
    assert(dtype.getClass() == H5T_STRING);
    assert(handle.getSpace().getSimpleExtentNdims() == 0);

    if (dtype.isVariableStr()) {
        const auto& dspace = handle.getSpace(); // don't set as temporary in Reclaim constructor below, otherwise it gets destroyed and the ID invalidated.
        const auto& plist = H5::DSetMemXferPropList::DEFAULT;
        char* vptr;
        handle.read(&vptr, dtype);
        [[maybe_unused]] ReclaimVlsMemory deletor(dtype.getId(), dspace.getId(), plist.getId(), &vptr);
        if (vptr == NULL) {
            throw std::runtime_error("detected a NULL pointer for a variable length string in '" + get_name(handle) + "'");
        }
        std::string output(vptr);
        return output;

    } else {
        size_t fixed_length = dtype.getSize();
        std::vector<char> buffer(fixed_length);
        handle.read(buffer.data(), dtype);
        return std::string(buffer.begin(), buffer.begin() + strnlen(buffer.data(), fixed_length));
    }
}

/**
 * @param attr Handle to a scalar string attribute.
 * @return The attribute as a string.
 */
inline std::string read_scalar_string(const H5::Attribute& attr) {
    auto dtype = attr.getDataType();
    assert(dtype.getClass() == H5T_STRING);
    assert(handle.getSpace().getSimpleExtentNdims() == 0);

    // Unfortunately, we can't just do 'std::string output; attr.read(dtype, output);', 
    // as we need to catch NULL pointers in the variable case.

    if (dtype.isVariableStr()) {
        auto mspace = attr.getSpace(); // don't set as a temporary in the Reclaim constructor, as it will be deleted and its ID invalidated.
        char* buffer = NULL;
        attr.read(dtype, &buffer);
        [[maybe_unused]] ReclaimVlsMemory deletor(dtype.getId(), mspace.getId(), H5P_DEFAULT, &buffer);
        if (buffer == NULL) {
            throw std::runtime_error("detected a NULL pointer for a variable length string attribute");
        }
        return std::string(buffer);

    } else {
        size_t len = dtype.getSize();
        std::vector<char> buffer(len);
        attr.read(dtype, buffer.data());
        auto ptr = buffer.data();
        return std::string(ptr, ptr + find_string_length(ptr, len));
    }
}

}

}

#endif
