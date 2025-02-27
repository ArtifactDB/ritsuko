#ifndef RITSUKO_HDF5_STREAM_1D_DATASET_HPP
#define RITSUKO_HDF5_STREAM_1D_DATASET_HPP

#include "H5Cpp.h"

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

#include "../pick_1d_block_size.hpp"
#include "../get_1d_length.hpp"
#include "../get_name.hpp"
#include "../_strings.hpp"
#include "define_pointer_datatype.hpp"

namespace ritsuko {

namespace hdf5 {

namespace vls {

template<typename Start_, typename Size_>
class Stream1dDataset {
public:
    Stream1dDataset(const H5::DataSet* pointers, const H5::DataSet* concatenated, hsize_t length, hsize_t buffer_size) : 
        p_dset(pointers), 
        s_dset(concatenated),
        p_full_length(length), 
        s_full_length(get_1d_length(s_dset->getSpace(), false)),
        p_block_size(pick_1d_block_size(p_dset->getCreatePlist(), p_full_length, buffer_size)),
        p_mspace(1, &p_block_size),
        p_dspace(1, &p_full_length),
        s_dspace(1, &s_full_length),
        p_dtype(define_pointer_datatype<Start_, Size_>())
    {
        p_buffer.resize(p_block_size);
        final_buffer.resize(p_block_size);
    }

    /**
     * Overloaded constructor where the length is automatically determined.
     *
     * @param ptr Pointer to a HDF5 dataset handle.
     * @param buffer_size Size of the buffer for holding streamed blocks of values.
     */
    Stream1dDataset(const H5::DataSet* pointers, const H5::DataSet* concatenated, hsize_t buffer_size) : 
        Stream1dDataset(pointers, concatenated, get_1d_length(pointers->getSpace(), false), buffer_size) 
    {}

public:
    /**
     * @return String at the current position of the stream.
     */
    std::string get() {
        while (consumed >= available) {
            consumed -= available;
            load(); 
        }
        return final_buffer[consumed];
    }

    /**
     * @return String at the current position of the stream.
     * Unlike `get()`, this avoids a copy by directly acquiring the string,
     * but it invalidates all subsequent `get()` and `steal()` requests until `next()` is called.
     */
    std::string steal() {
        while (consumed >= available) {
            consumed -= available;
            load(); 
        }
        return std::move(final_buffer[consumed]);
    }

    /**
     * Advance to the next position of the stream.
     *
     * @param jump Number of positions by which to advance the stream.
     */
    void next(size_t jump = 1) {
        consumed += jump;
    }

    /**
     * @return Length of the dataset.
     */
    hsize_t length() const {
        return p_full_length;
    }

    /**
     * @return Current position on the stream.
     */
    hsize_t position() const {
        return consumed + last_loaded;
    }

private:
    const H5::DataSet* p_dset;
    const H5::DataSet* s_dset;
    hsize_t p_full_length, s_full_length;
    hsize_t p_block_size;
    H5::DataSpace p_mspace, p_dspace;
    H5::DataSpace s_mspace, s_dspace;

    H5::DataType p_dtype;
    std::vector<Pointer<Start_, Size_> > p_buffer;
    std::vector<unsigned char> s_buffer;
    std::vector<std::string> final_buffer;

    hsize_t last_loaded = 0;
    hsize_t consumed = 0;
    hsize_t available = 0;

    void load() {
        if (last_loaded >= p_full_length) {
            throw std::runtime_error("requesting data beyond the end of the dataset at '" + get_name(*p_dset) + "'");
        }
        available = std::min(p_full_length - last_loaded, p_block_size);

        constexpr hsize_t zero = 0;
        p_mspace.selectHyperslab(H5S_SELECT_SET, &available, &zero);
        p_dspace.selectHyperslab(H5S_SELECT_SET, &available, &last_loaded);
        s_dspace.selectNone();
        p_dset->read(p_buffer.data(), p_dtype, p_mspace, p_dspace);

        for (size_t i = 0; i < available; ++i) {
            const auto& val = p_buffer[i];
            hsize_t start = val.start;
            hsize_t count = val.size;
            if (start > s_full_length || start + count > s_full_length) {
                throw std::runtime_error("pointers for variable length dataset at '" + get_name(*p_dset) + "' are out of range relative to concatenated dataset at '" + get_name(*s_dset) + "'");
            }

            auto& curstr = final_buffer[i];
            curstr.clear();

            if (count) {
                // Don't attempt to batch these reads as we aren't guaranteed
                // that they are non-overlapping or ordered. Hopefully HDF5 is
                // keeping enough things in cache for repeated reads.
                s_mspace.setExtentSimple(1, &count);
                s_mspace.selectAll();
                s_dspace.selectHyperslab(H5S_SELECT_SET, &count, &start);
                s_buffer.resize(count);
                s_dset->read(s_buffer.data(), H5::PredType::NATIVE_UCHAR, s_mspace, s_dspace);
                const char* text_ptr = reinterpret_cast<const char*>(s_buffer.data());
                curstr.insert(curstr.end(), text_ptr, text_ptr + find_string_length(text_ptr, count));
            }
        }

        last_loaded += available;
    }
};

}

}

}

#endif
