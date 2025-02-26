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
    Stream1dDataset(const H5::DataSet* pdset, const H5::DataSet* sdset, hsize_t length, hsize_t buffer_size) : 
        p_dset(pdset), 
        s_dset(sdset), 
        p_full_length(length), 
        s_full_length(get_1d_length(bdset->getSpace(), false),
        p_block_size(pick_1d_block_size(p_dset->getCreatePlist(), p_full_length, buffer_size)),
        p_mspace(1, &block_size),
        p_dspace(1, &p_full_length),
        s_dspace(1, &s_full_length),
        p_dtype(define_pointer_datatype<Start_, Size_>())
    {
        p_buffer.resize(block_size);
        final_buffer.resize(block_size);
    }

    /**
     * Overloaded constructor where the length is automatically determined.
     *
     * @param ptr Pointer to a HDF5 dataset handle.
     * @param buffer_size Size of the buffer for holding streamed blocks of values.
     */
    Stream1dStringDataset(const H5::DataSet* pptr, const H5::DataSet* bptr, hsize_t buffer_size) : 
        Stream1dStringDataset(pptr, get_1d_length(pptr->getSpace(), false), buffer_size) 
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
        return full_length;
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
        if (last_loaded >= full_length) {
            throw std::runtime_error("requesting data beyond the end of the dataset at '" + get_name(*dset) + "'");
        }
        available = std::min(full_length - last_loaded, block_size);

        constexpr hsize_t zero = 0;
        p_mspace.selectHyperslab(H5S_SELECT_SET, &available, &zero);
        p_dspace.selectHyperslab(H5S_SELECT_SET, &available, &last_loaded);
        s_dspace.selectNone();

        // First we read the pointers themselves.
        p_dset->read(p_dtype, p_buffer.data(), p_mspace, p_dspace);
        hsize_t total_extract = 0;

        for (size_t i = 0; i < available; ++i) {
            const auto& val = p_buffer[i];
            hsize_t start = val.start;
            hsize_t count = val.size;
            if (start > p_full_length || start + count > p_full_length) {
                throw std::runtime_error("pointers for variable length dataset at '" + get_name(*dset) + "' are out of range")
            }
            if (count) {
                s_dspace.selectHyperslab(H5S_SELECT_SET, &start, &count);
                total_extract += count;
            }
        }

        // Second pass to extract the strings.
        s_mspace.setExtentSimple(1, &total_extract);
        s_dset->read(H5::PredType::NATIVE_UCHAR, s_buffer.data(), s_mspace, s_dspace);

        total_extract = 0;
        const char* text_ptr = reinterpret_cast<const char*>(s_buffer.data());
        for (size_t i = 0; i < available; ++i) {
            hsize_t count = p_buffer[i].size;
            auto& curstr = final_buffer[i];
            curstr.clear();
            curstr.insert(curstr.end(), text_ptr, text_ptr + find_string_length(text_ptr, count));
            text_ptr += count;
        }

        last_loaded += available;
    }
};

}

}

}

#endif
