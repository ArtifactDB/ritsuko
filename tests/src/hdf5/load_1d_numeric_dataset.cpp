#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ritsuko/hdf5/load_1d_numeric_dataset.hpp"
#include "utils.h"

#include <cstdint>
#include <numeric>

TEST(Hdf5Load1dNumericDataset, Basic) {
    const char* path = "TEST-load-numeric.h5";

    std::vector<int> example(19191);
    std::iota(example.begin(), example.end(), 1);

    hsize_t chunk_size = 471;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, H5::PredType::NATIVE_UINT16, chunk_size);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    std::vector<int> counterexample(example.size()); 
    ritsuko::hdf5::load_1d_numeric_dataset<int32_t>(
        dhandle, 
        example.size(), 
        5000, 
        [&](const int32_t* d, size_t s, size_t l) {
            std::copy(d, d + l, counterexample.begin() + s);
        }
    );

    EXPECT_EQ(example, counterexample);
}

TEST(Hdf5Load1dNumericAttribute, basic) {
    const char* path = "TEST-load-numeric.h5";

    std::vector<int> example(19);
    std::iota(example.begin(), example.end(), 1);

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createGroup("whee");
        hsize_t len = example.size();
        H5::DataSpace dspace(1, &len);
        auto ahandle = dhandle.createAttribute("foo", H5::PredType::NATIVE_INT8, dspace);
        ahandle.write(H5::PredType::NATIVE_INT, example.data());
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openGroup("whee");
    auto ahandle = dhandle.openAttribute("foo");
    auto counterexample = ritsuko::hdf5::load_1d_numeric_attribute<int32_t>(ahandle, example.size());

    std::vector<int> countercopy(counterexample.begin(), counterexample.end());
    EXPECT_EQ(example, countercopy);
}
