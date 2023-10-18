#ifndef RITSUKO_TEST_UTILS_H
#define RITSUKO_TEST_UTILS_H

#include <vector>
#include <type_traits>
#include <stdexcept>

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

    if constexpr(std::is_same<T, int>::value) {
        dhandle.write(values.data(), H5::PredType::NATIVE_INT);
    } else if constexpr(std::is_same<T, double>::value) {
        dhandle.write(values.data(), H5::PredType::NATIVE_DOUBLE);
    } else {
        throw std::runtime_error("unknown type!");
    }

    return dhandle;
}

#endif
