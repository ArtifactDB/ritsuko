#ifndef RITSUKO_HDF5_VLS_TEST_UTILS_H
#define RITSUKO_HDF5_VLS_TEST_UTILS_H

#include <vector>
#include <type_traits>
#include <stdexcept>

#include "ritsuko/hdf5/vls/define_pointer_datatype.hpp"

template<typename T>
H5::DataSet create_vls_pointer_dataset(const H5::Group& parent, const std::string& name, const std::vector<ritsuko::hdf5::vls::Pointer<T, T> >& values, const H5::DataType& dtype, hsize_t compress_chunk = 0) {
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
    auto input_type = ritsuko::hdf5::vls::define_pointer_datatype<T, T>();
    dhandle.write(values.data(), input_type);

    return dhandle;
}

#endif
