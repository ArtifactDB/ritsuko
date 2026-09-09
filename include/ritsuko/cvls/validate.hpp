#ifndef RITSUKO_CVLS_VALIDATE_HPP
#define RITSUKO_CVLS_VALIDATE_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <limits>

#include "H5Cpp.h"

#include "../hdf5/get_name.hpp"
#include "../hdf5/IterateChunks.hpp"
#include "../hdf5/mock_contiguous_chunks.hpp"

#include "Pointer.hpp"

/**
 * @file validate.hpp
 * @brief Helper functions to validate compressed VLS datasets.
 */

namespace ritsuko {

namespace cvls {

/**
 * @cond
 */
template<typename Offset_, typename Length_>
inline void validate_pointers(const H5::DataSet& handle) {
    static_assert(std::is_integral<Offset_>::value);
    static_assert(std::is_integral<Length_>::value);

    if (handle.getTypeClass() != H5T_COMPOUND) {
        throw std::runtime_error("expected a compound datatype for a compressed VLS pointer dataset at '" + hdf5::get_name(handle) + "'");
    }

    try {
        validate_pointer_datatype(handle.getCompType(), std::numeric_limits<Offset_>::digits, std::numeric_limits<Length_>::digits);
    } catch (std::exception& e) {
        throw std::runtime_error("incorrect type for a compressed VLS pointer dataset at '" + hdf5::get_name(handle) + "; " + std::string(e.what()));
    }
}
/**
 * @endcond
 */

/**
 * Validate the pointer dataset for a compressed VLS scalar. 
 * An error is thrown if the datatype is not consistent with the expected precision of the `Pointer` types, 
 * or if the pointer is out of range of the associated heap dataset.
 *
 * @tparam Offset_ Unsigned integer type for the starting offset on the heap, see `Pointer::offset`.
 * @tparam Length_ Unsigned integer type for the length of the string, see `Pointer::length`.
 *
 * @param handle Handle to a scalar HDF5 dataset. 
 * @param heap_length Length of the heap dataset. 
 */
template<typename Offset_, typename Length_>
inline void validate_scalar_pointer(const H5::DataSet& handle, hsize_t heap_length) {
    validate_pointers<Offset_, Length_>(handle);

    auto dtype = define_pointer_datatype<Offset_, Length_>();
    Pointer<Offset_, Length_> val;
    handle.read(&val, dtype);

    hsize_t start = val.offset;
    hsize_t count = val.length;
    if (start > heap_length || start + count > heap_length) {
        throw std::runtime_error("VLS array pointers at '" + hdf5::get_name(handle) + "' are out of range of the heap");
    }
}


/**
 * Validate the pointer dataset for a 1-dimensional compressed VLS array. 
 * An error is thrown if the datatype is not consistent with the expected precision of the `Pointer` types,
 * or if any pointers are out of range of the associated heap dataset.
 *
 * @tparam Offset_ Unsigned integer type for the starting offset on the heap, see `Pointer::offset`.
 * @tparam Length_ Unsigned integer type for the length of the string, see `Pointer::length`.
 *
 * @param handle Handle to a 1-dimensional HDF5 dataset. 
 * @param full_length Length of the dataset as a 1-dimensional vector.
 * @param heap_length Length of the heap dataset. 
 */
template<typename Offset_, typename Length_>
inline void validate_1d_pointers(const H5::DataSet& handle, hsize_t full_length, hsize_t heap_length) {
    validate_pointers<Offset_, Length_>(handle);

    const auto& plist = handle.getCreatePlist();
    hsize_t block_size = 0;
    if (plist.getLayout() == H5D_CHUNKED) {
        plist.getChunk(1, &block_size);
    } else {
        // Hard-coding the mock chunk size for non-chunked datasets.
        block_size = std::min(full_length, static_cast<hsize_t>(10000));
    }

    H5::DataSpace mspace(1, &block_size), dspace(1, &full_length);
    std::vector<Pointer<Offset_, Length_> > buffer(block_size);
    auto dtype = define_pointer_datatype<Offset_, Length_>();

    hsize_t i = 0;
    while (i < full_length) {
        auto available = std::min(full_length - i, block_size);
        constexpr hsize_t zero = 0;
        mspace.selectHyperslab(H5S_SELECT_SET, &available, &zero);
        dspace.selectHyperslab(H5S_SELECT_SET, &available, &i);

        handle.read(buffer.data(), dtype, mspace, dspace);
        for (hsize_t j = 0; j < available; ++j) {
            const auto& val = buffer[j];
            hsize_t start = val.offset;
            hsize_t count = val.length;
            if (start > heap_length || start + count > heap_length) {
                throw std::runtime_error("VLS array pointers at '" + hdf5::get_name(handle) + "' are out of range of the heap");
            }
        }

        i += available;
    }
}

/**
 * Check that the pointers for an N-dimensional compressed VLS array is valid.
 * An error is thrown if the datatype is not consistent with the expected precision,
 * or if any pointers are out of range of the associated heap dataset.
 *
 * @tparam Offset_ Unsigned integer type for the starting offset on the heap, see `Pointer::offset`.
 * @tparam Length_ Unsigned integer type for the length of the string, see `Pointer::length`.
 *
 * @param handle Handle to a non-scalar HDF5 dataset. 
 * @param dimensions Dimensions of the dataset. 
 * This should be non-empty.
 * @param heap_length Length of the heap dataset. 
 */
template<typename Offset_, typename Length_>
void validate_nd_pointers(const H5::DataSet& handle, const std::vector<hsize_t>& dimensions, hsize_t heap_length) {
    validate_pointers<Offset_, Length_>(handle);

    std::vector<hsize_t> chunk_dims;
    const auto ndim = dimensions.size();
    const auto& plist = handle.getCreatePlist();
    if (plist.getLayout() == H5D_CHUNKED) {
        chunk_dims.resize(dimensions.size());
        plist.getChunk(ndim, chunk_dims.data());
    } else {
        // Hard-coding this to save ourselves an argument.
        chunk_dims = hdf5::mock_contiguous_chunks(dimensions, 10000);
    }

    hdf5::IterateChunks iter(dimensions, chunk_dims);
    H5::DataSpace fspace(ndim, dimensions.data());
    H5::DataSpace mspace(ndim, iter.chunk_dimensions().data());

    std::vector<Pointer<Offset_, Length_> > buffer;
    auto dtype = define_pointer_datatype<Offset_, Length_>();

    while (iter.advance()) {
        const auto& curcount = iter.counts();
        mspace.setExtentSimple(ndim, curcount.data());
        fspace.selectHyperslab(H5S_SELECT_SET, curcount.data(), iter.starts().data());
        buffer.resize(mspace.getSimpleExtentNpoints());

        handle.read(buffer.data(), dtype, mspace, fspace);
        for (const auto& val : buffer) {
            hsize_t start = val.offset;
            hsize_t count = val.length;
            if (start > heap_length || start + count > heap_length) {
                throw std::runtime_error("VLS array pointers at '" + hdf5::get_name(handle) + "' are out of range of the heap");
            }
        }
    }
}

/**
 * Validate a HDF5 dataset containing the compressed VLS heap.
 * An error is thrown if the dataset is not 1-dimensional or does not contain unsigned 8-bit integers.
 *
 * @param handle Handle to a HDF5 dataset.
 */
inline void validate_heap(const H5::DataSet& handle) {
    if (handle.getTypeClass() != H5T_INTEGER) {
        throw std::runtime_error("expected an integer datatype for the compressed VLS heap at '" + hdf5::get_name(handle) + "'");
    }
    if (hdf5::exceeds_integer_limit(handle.getIntType(), 8, false)) {
        throw std::runtime_error("expected 8-bit unsigned integers for the compressed VLS heap at '" + hdf5::get_name(handle) + "'");
    }
    if (handle.getSpace().getSimpleExtentNdims() != 1) {
        throw std::runtime_error("expected a 1-dimensional dataset for the compressed VLS heap at '" + hdf5::get_name(handle) + "'");
    }
}

}

}

#endif
