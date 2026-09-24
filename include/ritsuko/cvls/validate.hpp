#ifndef RITSUKO_CVLS_VALIDATE_HPP
#define RITSUKO_CVLS_VALIDATE_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <limits>
#include <cassert>
#include <algorithm>

#include "H5Cpp.h"
#include "sanisizer/sanisizer.hpp"

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
 * Validate the pointer dataset for a compressed VLS scalar. 
 * An error is thrown if the datatype is not consistent with the expected precision of the `Pointer` types, 
 * or if the pointer is out of range of the associated heap dataset.
 *
 * @tparam Offset_ Unsigned integer type for the starting offset on the heap, see `Pointer::offset`.
 * @tparam Length_ Unsigned integer type for the length of the string, see `Pointer::length`.
 *
 * @param data A HDF5 dataset. 
 * It is assumed that this dataset is scalar.
 * @param heap_length Length of the heap dataset, see `validate_heap()`.
 */
template<typename Offset_, typename Length_>
inline void validate_scalar_pointer(const H5::DataSet& data, hsize_t heap_length) {
    validate_pointer_datatype(data, std::numeric_limits<Offset_>::digits, std::numeric_limits<Length_>::digits);
    assert(data.getSpace().getSimpleExtentNdims() == 0);

    auto dtype = define_pointer_datatype<Offset_, Length_>();
    Pointer<Offset_, Length_> val;
    data.read(&val, dtype);

    if (is_Pointer_out_of_range(val, heap_length)) {
        throw std::runtime_error("compressed VLS array pointer at '" + hdf5::get_name(data) + "' is out of range of the heap");
    }
}

/**
 * @brief Options for `validate_1d_pointers()`.
 */
struct Validate1dPointersOptions {
    /**
     * Size of the streaming chunks (in terms of the number of elements) to use for contiguous HDF5 datasets.
     * This is ignored for chunked datasets where the streaming chunk size is identical to the dataset chunk size. 
     */
    hsize_t contiguous_chunk_size = sanisizer::cap<hsize_t>(10000);
};

/**
 * Validate the pointer dataset for a 1-dimensional compressed VLS array. 
 * An error is thrown if the datatype is not consistent with the expected precision of the `Pointer` types,
 * or if any pointers are out of range of the associated heap dataset.
 *
 * @tparam Offset_ Unsigned integer type for the starting offset on the heap, see `Pointer::offset`.
 * @tparam Length_ Unsigned integer type for the length of the string, see `Pointer::length`.
 *
 * @param data A HDF5 dataset. 
 * It is assumed that this dataset is 1-dimensional.
 * @param full_length Length of the dataset, i.e., the extent of its sole dimension.
 * @param heap_length Length of the heap dataset, see `validate_heap()`.
 * @param options Further options.
 */
template<typename Offset_, typename Length_>
inline void validate_1d_pointers(const H5::DataSet& data, hsize_t full_length, hsize_t heap_length, const Validate1dPointersOptions& options) {
    validate_pointer_datatype(data, std::numeric_limits<Offset_>::digits, std::numeric_limits<Length_>::digits);
    assert(data.getSpace().getSimpleExtentNdims() == 1);

    const auto& plist = data.getCreatePlist();
    hsize_t block_size = 0;
    if (plist.getLayout() == H5D_CHUNKED) {
        plist.getChunk(1, &block_size);
    } else {
        block_size = std::min(full_length, options.contiguous_chunk_size);
    }

    H5::DataSpace mspace(1, &block_size), dspace(1, &full_length);
    auto buffer = sanisizer::create<std::vector<Pointer<Offset_, Length_> > >(block_size);
    auto dtype = define_pointer_datatype<Offset_, Length_>();

    hsize_t i = 0;
    while (i < full_length) {
        const auto available = sanisizer::min(full_length - i, block_size);
        constexpr hsize_t zero = 0;
        mspace.selectHyperslab(H5S_SELECT_SET, &available, &zero);
        dspace.selectHyperslab(H5S_SELECT_SET, &available, &i);

        data.read(buffer.data(), dtype, mspace, dspace);
        for (I<decltype(available)> j = 0; j < available; ++j) {
            const auto& val = buffer[j];
            if (is_Pointer_out_of_range(val, heap_length)) {
                throw std::runtime_error("compressed VLS array pointers at '" + hdf5::get_name(data) + "' are out of range of the heap");
            }
        }

        i += available;
    }
}

/**
 * @brief Options for `validate_nd_pointers()`.
 */
struct ValidateNdPointersOptions {
    /**
     * Size of the streaming chunks (in terms of the number of elements) to use for contiguous HDF5 datasets.
     * This is ignored for chunked datasets where the streaming chunk dimensions are identical to the dataset chunk dimensions.
     */
    hsize_t contiguous_chunk_size = sanisizer::cap<hsize_t>(10000);
};

/**
 * Check that the pointers for an N-dimensional compressed VLS array is valid.
 * An error is thrown if the datatype is not consistent with the expected precision,
 * or if any pointers are out of range of the associated heap dataset.
 *
 * @tparam Offset_ Unsigned integer type for the starting offset on the heap, see `Pointer::offset`.
 * @tparam Length_ Unsigned integer type for the length of the string, see `Pointer::length`.
 *
 * @param data A non-scalar HDF5 dataset. 
 * It is assumed that this dataset has at least 1 dimension.
 * @param dimensions Dimensions of the dataset. 
 * This should be non-empty.
 * @param heap_length Length of the heap dataset, see `validate_heap()`.
 * @param options Further options.
 */
template<typename Offset_, typename Length_>
void validate_nd_pointers(const H5::DataSet& data, const std::vector<hsize_t>& dimensions, hsize_t heap_length, const ValidateNdPointersOptions& options) {
    validate_pointer_datatype(data, std::numeric_limits<Offset_>::digits, std::numeric_limits<Length_>::digits);
    assert(data.getSpace().getSimpleExtentNdims() > 0);

    // Cast of 'ndim' to 'int' is implicitly safe if the assertion holds.
    const auto ndim = dimensions.size();
    assert(sanisizer::is_equal(ndim, data.getSpace().getSimpleExtentNdims()));

    std::vector<hsize_t> chunk_dims;
    const auto& plist = data.getCreatePlist();
    if (plist.getLayout() == H5D_CHUNKED) {
        chunk_dims.resize(ndim); // No need to check this, dimensions is of the same type as chunk_dims.
        plist.getChunk(ndim, chunk_dims.data());
    } else {
        chunk_dims = hdf5::mock_contiguous_chunks(dimensions, options.contiguous_chunk_size);
    }

    hdf5::IterateChunks iter(dimensions, chunk_dims);
    H5::DataSpace fspace(ndim, dimensions.data());
    H5::DataSpace mspace(ndim, iter.chunk_dimensions().data());
    auto buffer = sanisizer::create<std::vector<Pointer<Offset_, Length_> > >(mspace.getSimpleExtentNpoints());
    auto dtype = define_pointer_datatype<Offset_, Length_>();

    while (iter.advance()) {
        const auto& curcount = iter.counts();
        mspace.setExtentSimple(ndim, curcount.data());
        fspace.selectHyperslab(H5S_SELECT_SET, curcount.data(), iter.starts().data());

        data.read(buffer.data(), dtype, mspace, fspace);
        const auto available = mspace.getSimpleExtentNpoints();
        for (I<decltype(available)> i = 0; i < available; ++i) {
            const auto& val = buffer[i];
            if (is_Pointer_out_of_range(val, heap_length)) {
                throw std::runtime_error("compressed VLS array pointers at '" + hdf5::get_name(data) + "' are out of range of the heap");
            }
        }
    }
}

/**
 * Validate a HDF5 dataset containing the compressed VLS heap.
 * An error is thrown if the dataset is not 1-dimensional or does not contain unsigned 8-bit integers.
 *
 * @param data A HDF5 dataset.
 * It may have any shape and its datatype may be of any class.
 *
 * @return Extent of the sole dimension of `data`, typically for use as `heap_length` in `validate_nd_pointers()` and related functions.
 */
inline hsize_t validate_heap(const H5::DataSet& data) {
    if (data.getTypeClass() != H5T_INTEGER) {
        throw std::runtime_error("expected an integer datatype for the compressed VLS heap at '" + hdf5::get_name(data) + "'");
    }
    if (hdf5::exceeds_integer_limit(data.getIntType(), 8, false)) {
        throw std::runtime_error("expected 8-bit unsigned integers for the compressed VLS heap at '" + hdf5::get_name(data) + "'");
    }
    auto dspace = data.getSpace();
    if (dspace.getSimpleExtentNdims() != 1) {
        throw std::runtime_error("expected a 1-dimensional dataset for the compressed VLS heap at '" + hdf5::get_name(data) + "'");
    }
    hsize_t len;
    dspace.getSimpleExtentDims(&len);
    return len;
}

}

}

#endif
