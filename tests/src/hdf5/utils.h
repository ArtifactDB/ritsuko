#ifndef RITSUKO_HDF5_TEST_UTILS_H
#define RITSUKO_HDF5_TEST_UTILS_H

#include <vector>
#include <type_traits>
#include <stdexcept>

#include "ritsuko/hdf5/as_numeric_datatype.hpp"

template<typename T>
H5::DataSet create_dataset(const H5::Group& parent, const std::string& name, const std::vector<T>& values, const H5::DataType& dtype, hsize_t compress_chunk = 0) {
    hsize_t len = values.size();
    H5::DataSpace dspace(1, &len);

    H5::DSetCreatPropList cplist;
    if (compress_chunk) {
        cplist.setChunk(1, &compress_chunk);
        cplist.setDeflate(8);
    } else {
        cplist = H5::DSetCreatPropList::DEFAULT;
    }

    auto dhandle = parent.createDataSet(name, dtype, dspace, cplist);
    dhandle.write(values.data(), ritsuko::hdf5::as_numeric_datatype<T>());
    return dhandle;
}

inline H5::DataSet create_dataset(
    const H5::Group& parent, 
    const std::string& name, 
    const std::vector<std::string>& values, 
    bool variable = false, 
    hsize_t compress_chunk = 0)
{
    hsize_t len = values.size();
    H5::DataSpace dspace(1, &len);

    H5::DSetCreatPropList cplist;
    if (compress_chunk) {
        cplist.setChunk(1, &compress_chunk);
        cplist.setDeflate(6);
    } else {
        cplist = H5::DSetCreatPropList::DEFAULT;
    }

    if (!variable) {
        size_t maxlen = 1;
        for (const auto& v : values) {
            if (v.size() > maxlen) {
                maxlen = v.size();
            }
        }

        std::vector<char> buffer(maxlen * values.size());
        for (size_t v = 0; v < values.size(); ++v) {
            const auto& current = values[v];
            std::copy(current.begin(), current.end(), buffer.data() + v * maxlen);
        }

        H5::StrType stype(0, maxlen);
        auto dhandle = parent.createDataSet(name, stype, dspace, cplist);
        dhandle.write(buffer.data(), stype);
        return dhandle;

    } else {
        std::vector<const char*> ptrs;
        ptrs.reserve(values.size());
        for (const auto& v : values) {
            ptrs.push_back(v.c_str());
        }

        H5::StrType stype(H5::PredType::C_S1, H5T_VARIABLE); 
        auto dhandle = parent.createDataSet(name, stype, dspace, cplist);
        dhandle.write(ptrs.data(), stype);
        return dhandle;
    }
}


#endif
