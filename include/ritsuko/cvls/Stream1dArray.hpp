#ifndef RITSUKO_CVLS_STREAM_1D_ARRAY_HPP
#define RITSUKO_CVLS_STREAM_1D_ARRAY_HPP

#include "H5Cpp.h"

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

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
 *
 * This streams in a 1-dimensional compressed VLS array in chunks.
 * Callers can then iterate over the individual strings.
 */
template<typename Offset_, typename Length_>
class Stream1dArray {
public:
    /**
     * @param pointers Pointer to a 1-dimensional HDF5 dataset containing the compressed VLS pointers.
     * This dataset should satisfy `validate_1d_pointers()`.
     * @param length Length of the `pointers` dataset. 
     * @param heap Pointer to a 1-dimensional HDF5 dataset containing the compressed VLS heap.
     * This dataset should satisfy `validate_heap()`.
     */
    Stream1dArray(const H5::DataSet& pointers, hsize_t length, const H5::DataSet& heap) : 
        my_pointers(pointers), 
        my_heap(heap),
        my_pointer_full_length(length), 
        my_heap_full_length([&]{
            hsize_t output;
            my_heap.getSpace().getSimpleExtentDims(&output);
            return output;
        }()),
        my_pointer_block_size([&]{
            hsize_t output;
            const auto& plist = pointers.getCreatePlist();
            if (plist.getLayout() == H5D_CHUNKED) {
                plist.getChunk(1, &output);
            } else {
                output = std::min(static_cast<hsize_t>(10000), my_pointer_full_length);
            }
            return output;
        }()),
        my_pointer_mspace(1, &my_pointer_block_size),
        my_pointer_dspace(1, &my_pointer_full_length),
        my_heap_dspace(1, &my_heap_full_length),
        my_pointer_dtype(define_pointer_datatype<Offset_, Length_>()),
        my_pointer_buffer(my_pointer_block_size),
        my_final_buffer(my_pointer_block_size)
    {}

public:
    /**
     * Load the contents of the next chunk in the dataset.
     * On return, this is now the "current" chunk.
     *
     * @return Number of elements loaded in the current chunk.
     * If zero is returned, the dataset traversal is complete.
     */
    hsize_t load() {
        my_last_loaded += my_available;
        my_available = std::min(my_pointer_full_length - my_last_loaded, my_pointer_block_size);
        if (my_available == 0) {
            return 0;
        }

        constexpr hsize_t zero = 0;
        my_pointer_mspace.selectHyperslab(H5S_SELECT_SET, &my_available, &zero);
        my_pointer_dspace.selectHyperslab(H5S_SELECT_SET, &my_available, &my_last_loaded);
        my_heap_dspace.selectNone();
        my_pointers.read(my_pointer_buffer.data(), my_pointer_dtype, my_pointer_mspace, my_pointer_dspace);

        for (size_t i = 0; i < my_available; ++i) {
            const auto& val = my_pointer_buffer[i];
            hsize_t start = val.offset;
            hsize_t count = val.length;
            if (start > my_heap_full_length || start + count > my_heap_full_length) {
                throw std::runtime_error("compressed VLS array pointers at '" + 
                    hdf5::get_name(my_pointers) +
                    "' are out of range of the heap at '" +
                    hdf5::get_name(my_heap) +
                    "'"
                );
            }

            auto& curstr = my_final_buffer[i];
            curstr.clear();

            if (count) {
                // Don't attempt to batch these reads as we aren't guaranteed
                // that they are non-overlapping or ordered. Hopefully HDF5 is
                // keeping enough things in cache for repeated reads.
                my_heap_mspace.setExtentSimple(1, &count);
                my_heap_mspace.selectAll();
                my_heap_dspace.selectHyperslab(H5S_SELECT_SET, &count, &start);
                my_heap_buffer.resize(count);
                my_heap.read(my_heap_buffer.data(), H5::PredType::NATIVE_UINT8, my_heap_mspace, my_heap_dspace);
                const char* text_ptr = reinterpret_cast<const char*>(my_heap_buffer.data());
                curstr.insert(curstr.end(), text_ptr, text_ptr + hdf5::strnlen(text_ptr, count));

                /*
                 * Is it generally portable to reinterpret_cast the bytes in a
                 * uint8_t array? I think so; according to the C standard,
                 * uint8_t is guaranteed to be exactly 8 bits
                 * (https://stackoverflow.com/questions/15039077/uint8-t-8-bits-guarantee),
                 * so a uint8_t value should have the same bit representation
                 * across all implementations that define the uint8_t type. If
                 * we save a byte to HDF5 as a UINT8 on one machine and read it
                 * back into to memory on another machine, we should recover
                 * the same bit pattern. Thus, the reinterpret_cast to a char*
                 * should yield the same bit pattern across machines, allowing
                 * us to portably interpret the array as a string following the
                 * ASCII/UTF-8 spec (which define each character in binary).
                 */
            }
        }

        return my_available;
    }

    /**
     * Get the contents of the current chunk.
     * This should only be called after `load()`.
     *
     * @return Pointer to an array containing the contents of the current chunk.
     * Only the first `X` elements should be accessed, where `X` is the return value of the most recent call to `load()`. 
     * Callers can freely modify the contents of this array.
     */
    std::string* contents() {
        return my_final_buffer.data();                
    }

    /**
     * @return Start position of the current chunk, i.e., the index of the first element in the chunk. 
     */
    hsize_t start() const {
        return my_last_loaded;
    }

private:
    const H5::DataSet& my_pointers;
    const H5::DataSet& my_heap;
    hsize_t my_pointer_full_length, my_heap_full_length;
    hsize_t my_pointer_block_size;
    H5::DataSpace my_pointer_mspace, my_pointer_dspace;
    H5::DataSpace my_heap_mspace, my_heap_dspace;

    H5::DataType my_pointer_dtype;
    std::vector<Pointer<Offset_, Length_> > my_pointer_buffer;
    std::vector<std::uint8_t> my_heap_buffer;
    std::vector<std::string> my_final_buffer;

    hsize_t my_last_loaded = 0;
    hsize_t my_available = 0;

};

}

}

#endif
