#ifndef RITSUKO_CVLS_STREAM_1D_ARRAY_HPP
#define RITSUKO_CVLS_STREAM_1D_ARRAY_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cassert>

#include "H5Cpp.h"
#include "sanisizer/sanisizer.hpp"

#include "../hdf5/get_name.hpp"
#include "../hdf5/strnlen.hpp"

#include "Pointer.hpp"

/**
 * @file Stream1dArray.hpp
 * @brief Stream a 1-dimensional compressed VLS array into memory.
 */

namespace ritsuko {

namespace cvls {

/**
 * @brief Stream a 1-dimensional compressed VLS array into memory.
 *
 * @tparam Offset_ Unsigned integer type for the starting offset on the heap, see `Pointer::offset`.
 * @tparam Length_ Unsigned integer type for the length of the string, see `Pointer::length`.
 * @tparam DataSetPointer_ Class of a pointer to a `H5::DataSet`.
 * This can be raw or smart depending on the caller's management of its lifetime.
 *
 * This streams in a 1-dimensional compressed VLS array in chunks.
 * Callers can then iterate over the individual strings.
 */
template<typename Offset_, typename Length_, typename DataSetPointer_ = const H5::DataSet*>
class Stream1dArray {
public:
    /**
     * @param pointers_ptr Pointer to a HDF5 dataset containing the compressed VLS pointers.
     * It is assumed that this dataset already satisfies `validate_1d_pointers()`.
     * It is also assumed that this dataset is 1-dimensional.
     * @param length Length of the `pointers_ptr` dataset, i.e., the extent of its sole dimension.
     * @param heap_ptr Pointer to a HDF5 dataset containing the compressed VLS heap.
     * It is assumed that this dataset already satisfies `validate_heap()`.
     */
    Stream1dArray(DataSetPointer_ pointers_ptr, hsize_t length, DataSetPointer_ heap_ptr) : 
        my_pointers_ptr(std::move(pointers_ptr)), 
        my_heap_ptr(std::move(heap_ptr)),
        my_pointer_full_length(length), 
        my_heap_full_length([&]{
            hsize_t output;
            my_heap_ptr->getSpace().getSimpleExtentDims(&output);
            return output;
        }()),
        my_pointer_block_size([&]{
            assert(my_pointers_ptr->getSpace().getSimpleExtentNdims() == 1);
            hsize_t output;
            const auto& plist = my_pointers_ptr->getCreatePlist();
            if (plist.getLayout() == H5D_CHUNKED) {
                plist.getChunk(1, &output);
            } else {
                // Hard-coding the upper bound to save ourselves from processing an extra argument.
                output = sanisizer::min(my_pointer_full_length, 10000);
            }
            return output;
        }()),
        my_pointer_mspace(1, &my_pointer_block_size),
        my_pointer_dspace(1, &my_pointer_full_length),
        my_heap_dspace(1, &my_heap_full_length),
        my_pointer_dtype(define_pointer_datatype<Offset_, Length_>()),
        my_pointer_buffer(sanisizer::cast<I<decltype(my_pointer_buffer.size())> >(my_pointer_block_size))
    {
        // Check that maximum allocation is possible, so we don't have to check casts for individual string lengths.
        sanisizer::cast<I<decltype(my_heap_buffer.size())> >(my_heap_full_length);
    }

public:
    /**
     * @return Size of each chunk, in terms of the number of elements.
     */
    hsize_t chunk_size() const {
        return my_pointer_block_size;
    }

    /**
     * Load the contents of the next chunk in the dataset.
     *
     * @param[out] buffer Pointer to an array of `chunk_size()`, where each entry is a valid `std::string`.
     * On output, this contains the contents of the current chunk in its first \f$X\f$ elements,
     * where \f$X\f$ is the return value of this method.
     *
     * @return Number of elements loaded in the current chunk.
     * If zero is returned, the dataset traversal is complete.
     */
    hsize_t load(std::string* buffer) {
        my_last_loaded += my_available;
        my_available = std::min(my_pointer_full_length - my_last_loaded, my_pointer_block_size);
        if (my_available == 0) {
            return 0;
        }

        constexpr hsize_t zero = 0;
        my_pointer_mspace.selectHyperslab(H5S_SELECT_SET, &my_available, &zero);
        my_pointer_dspace.selectHyperslab(H5S_SELECT_SET, &my_available, &my_last_loaded);
        my_heap_dspace.selectNone();
        my_pointers_ptr->read(my_pointer_buffer.data(), my_pointer_dtype, my_pointer_mspace, my_pointer_dspace);

        for (size_t i = 0; i < my_available; ++i) {
            const auto& val = my_pointer_buffer[i];
            if (is_pointer_out_of_range(val.offset, val.length, my_heap_full_length)) {
                throw std::runtime_error("compressed VLS array pointers at '" + 
                    hdf5::get_name(*my_pointers_ptr) +
                    "' are out of range of the heap at '" +
                    hdf5::get_name(*my_heap_ptr) +
                    "'"
                );
            }

            auto& curstr = buffer[i];
            curstr.clear();

            if (val.length) {
                // Casts are safe if the pointers are within range.
                const hsize_t start = val.offset;
                const hsize_t count = val.length;

                // Don't attempt to batch these reads as we aren't guaranteed
                // that they are non-overlapping or ordered. Hopefully HDF5 is
                // keeping enough things in cache for repeated reads.
                my_heap_mspace.setExtentSimple(1, &count);
                my_heap_mspace.selectAll();
                my_heap_dspace.selectHyperslab(H5S_SELECT_SET, &count, &start);
                my_heap_buffer.resize(count);
                my_heap_ptr->read(my_heap_buffer.data(), H5::PredType::NATIVE_UINT8, my_heap_mspace, my_heap_dspace);
                const char* text_ptr = reinterpret_cast<const char*>(my_heap_buffer.data());
                curstr.insert(curstr.end(), text_ptr, text_ptr + hdf5::strnlen(text_ptr, count));
            }
        }

        return my_available;
    }

    /**
     * Get the start position of the current chunk, i.e., the index of the first element in the chunk. 
     * That is, `buffer[j]` corresponds to the `start() + j`-th element of the dataset.
     * This should only be called after `load()`.
     *
     * @return Start position of the current chunk.
     */
    hsize_t start() const {
        return my_last_loaded;
    }

private:
    DataSetPointer_ my_pointers_ptr;
    DataSetPointer_ my_heap_ptr;
    hsize_t my_pointer_full_length, my_heap_full_length;
    hsize_t my_pointer_block_size;
    H5::DataSpace my_pointer_mspace, my_pointer_dspace;
    H5::DataSpace my_heap_mspace, my_heap_dspace;

    H5::DataType my_pointer_dtype;
    std::vector<Pointer<Offset_, Length_> > my_pointer_buffer;
    std::vector<std::uint8_t> my_heap_buffer;

    hsize_t my_last_loaded = 0;
    hsize_t my_available = 0;

};

}

}

#endif
