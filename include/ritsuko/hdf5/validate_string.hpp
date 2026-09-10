#ifndef RITSUKO_HDF5_VALIDATE_STRING_HPP
#define RITSUKO_HDF5_VALIDATE_STRING_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <cassert>

#include "H5Cpp.h"

#include "get_name.hpp"
#include "mock_contiguous_chunks.hpp"
#include "IterateChunks.hpp"
#include "ReclaimVlsMemory.hpp"

/**
 * @file validate_string.hpp
 * @brief Helper functions to validate strings.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * Check that a scalar string dataset is valid.
 * Currently, this involves checking that there are no `NULL` entries for variable-length string datatypes.
 * For fixed-width string datasets, this function is a no-op.
 *
 * @param data A HDF5 dataset.
 * It is assumed that this dataset is scalar.
 * It is also assumed that its datatype is of the string class.
 */
inline void validate_scalar_string(const H5::DataSet& data) {
    assert(data.getSpace().getSimpleExtentNdims() == 0);
    assert(data.getDataType().getClass() == H5T_STRING);

    auto dtype = data.getDataType();
    if (!dtype.isVariableStr()) {
        return;
    }

    char* vptr = NULL;
    data.read(&vptr, dtype);

    const auto& dspace = data.getSpace();
    const auto& plist = H5::DSetMemXferPropList::DEFAULT;
    [[maybe_unused]] ReclaimVlsMemory deletor(dtype.getId(), dspace.getId(), plist.getId(), &vptr);

    if (vptr == NULL) {
        throw std::runtime_error("detected a NULL pointer for a variable length string in '" + get_name(data) + "'");
    }
}

/**
 * Check that a 1-dimensional string dataset is valid.
 * Currently, this involves checking that there are no `NULL` entries for variable-length string datatypes.
 * For fixed-width string datasets, this function is a no-op.
 *
 * @param data A HDF5 dataset.
 * It is assumed that this dataset is 1-dimensional.
 * It is also assumed that its datatype is of the string class.
 * @param full_length Length of the dataset, i.e., the extent of its sole dimension.
 */
inline void validate_1d_strings(const H5::DataSet& data, hsize_t full_length) {
    assert(data.getSpace().getSimpleExtentNdims() == 1);
    assert(data.getDataType().getClass() == H5T_STRING);

    auto dtype = data.getDataType();
    if (!dtype.isVariableStr()) {
        return;
    }

    hsize_t block_size = 10000;
    const auto& plist = data.getCreatePlist();
    if (plist.getLayout() == H5D_CHUNKED) {
        plist.getChunk(1, &block_size);
    }

    H5::DataSpace mspace(1, &block_size), dspace(1, &full_length);
    std::vector<char*> buffer(block_size);

    for (hsize_t i = 0; i < full_length; i += block_size) {
        auto available = std::min(full_length - i, block_size);
        constexpr hsize_t zero = 0;
        mspace.selectHyperslab(H5S_SELECT_SET, &available, &zero);
        dspace.selectHyperslab(H5S_SELECT_SET, &available, &i);

        data.read(buffer.data(), dtype, mspace, dspace);

        const auto& plist = H5::DSetMemXferPropList::DEFAULT;
        [[maybe_unused]] ReclaimVlsMemory deletor(dtype.getId(), mspace.getId(), plist.getId(), buffer.data());
        for (hsize_t j = 0; j < available; ++j) {
            if (buffer[j] == NULL) {
                throw std::runtime_error("detected a NULL pointer for a variable length string in '" + get_name(data) + "'");
            }
        }
    }
}

/**
 * Check that an N-dimensional string dataset is valid.
 * Currently, this involves checking that there are no `NULL` entries for variable-length string datatypes.
 * For fixed-width string datasets, this function is a no-op.
 *
 * @param data A HDF5 dataset.
 * It is assumed that this dataset has at least 1 dimension.
 * It is also assumed that its datatype is of the string class.
 * @param dimensions Dimensions of the dataset.
 */
inline void validate_nd_strings(const H5::DataSet& data, const std::vector<hsize_t>& dimensions) {
    assert(data.getSpace().getSimpleExtentNdims() > 0);
    assert(data.getDataType().getClass() == H5T_STRING);

    auto stype = data.getDataType();
    if (!stype.isVariableStr()) {
        return;
    }

    std::vector<hsize_t> chunk_dims;
    const auto& plist = data.getCreatePlist();
    if (plist.getLayout() == H5D_CHUNKED) {
        chunk_dims.resize(dimensions.size());
        plist.getChunk(dimensions.size(), chunk_dims.data());
    } else {
        // Hard-coding this to save ourselves an argument.
        chunk_dims = mock_contiguous_chunks(dimensions, 10000);
    }

    IterateChunks iter(dimensions, chunk_dims);

    const auto ndim = dimensions.size();
    H5::DataSpace fspace(ndim, dimensions.data());
    H5::DataSpace mspace(ndim, iter.chunk_dimensions().data());
    std::vector<char*> buffer(mspace.getSimpleExtentNpoints());

    while (iter.advance()) {
        const auto& curcount = iter.counts();
        mspace.setExtentSimple(ndim, curcount.data());
        fspace.selectHyperslab(H5S_SELECT_SET, curcount.data(), iter.starts().data());

        data.read(buffer.data(), stype, mspace, fspace);
        const auto& plist = H5::DSetMemXferPropList::DEFAULT;
        [[maybe_unused]] ReclaimVlsMemory deleter(stype.getId(), mspace.getId(), plist.getId(), buffer.data());

        const std::size_t npts = mspace.getSimpleExtentNpoints();
        for (std::size_t i = 0; i < npts; ++i) {
            if (buffer[i] == NULL) {
                throw std::runtime_error("detected NULL pointer in a variable-length string dataset");
            }
        }
    }
}

/**
 * Check that a scalar string attribute is valid.
 * Currently, this involves checking that there are no `NULL` entries for variable-length string datatypes.
 * For fixed-width string attributes, this function is a no-op.
 *
 * @param attr A HDF5 attribute.
 * It is assumed that this attribute is scalar.
 * It is also assumed that its datatype is of the string class.
 */
inline void validate_scalar_string_attribute(const H5::Attribute& attr) {
    assert(attr.getSpace().getSimpleExtentNdims() == 0);
    assert(attr.getDataType().getClass() == H5T_STRING);

    auto dtype = attr.getDataType();
    if (!dtype.isVariableStr()) {
        return;
    }

    auto mspace = attr.getSpace();
    char* buffer;
    attr.read(dtype, &buffer);
    [[maybe_unused]] ReclaimVlsMemory deletor(dtype.getId(), mspace.getId(), H5P_DEFAULT, &buffer);
    if (buffer == NULL) {
        throw std::runtime_error("detected a NULL pointer for a variable length string attribute");
    }
}

/**
 * Check that a 1-dimensional string attribute is valid.
 * Currently, this involves checking that there are no `NULL` entries for variable-length string datatypes.
 * For fixed-width string attributes, this function is a no-op.
 *
 * @param attr Handle to the HDF5 string attribute.
 * It is assumed that this attribute is 1-dimensional.
 * It is also assumed that its datatype is of the string class.
 * @param full_length Length of the attribute, i.e., the extent of its sole dimension.
 */
inline void validate_1d_string_attribute(const H5::Attribute& attr, hsize_t full_length) {
    assert(attr.getSpace().getSimpleExtentNdims() == 1);
    assert(attr.getDataType().getClass() == H5T_STRING);

    auto dtype = attr.getDataType();
    if (!dtype.isVariableStr()) {
        return;
    }

    auto mspace = attr.getSpace();
    std::vector<char*> buffer(full_length);
    attr.read(dtype, buffer.data());
    [[maybe_unused]] ReclaimVlsMemory deletor(dtype.getId(), mspace.getId(), H5P_DEFAULT, buffer.data());
    for (hsize_t i = 0; i < full_length; ++i) {
        if (buffer[i] == NULL) {
            throw std::runtime_error("detected a NULL pointer for a variable length string attribute");
        }
    }
}

}

}

#endif
