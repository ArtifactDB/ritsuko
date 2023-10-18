#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/load_1d_string_dataset.hpp"
#include "utils.h"
#include <numeric>
#include <string>

TEST(Hdf5Load1dStringDataset, Fixed) {
    const char* path = "TEST-load-string.h5";

    std::vector<std::string> example(19191);
    for (size_t i = 0; i < example.size(); ++i) {
        example[i] = std::to_string(i);
    }

    hsize_t chunk_size = 471;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, false, chunk_size);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    std::vector<std::string> counterexample(example.size()); 
    ritsuko::hdf5::load_1d_string_dataset(
        dhandle, 
        example.size(), 
        5000, 
        [&](size_t i, const char* p, size_t l) {
            counterexample[i] = std::string(p, p + l);
        }
    );
}

TEST(Hdf5Load1dStringDataset, Variable) {
    const char* path = "TEST-load-string.h5";

    std::vector<std::string> example(19191);
    for (size_t i = 0; i < example.size(); ++i) {
        example[i] = std::to_string(i);
    }

    hsize_t chunk_size = 999;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", example, true, chunk_size);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    std::vector<std::string> counterexample(example.size()); 
    ritsuko::hdf5::load_1d_string_dataset(
        dhandle, 
        example.size(), 
        5000, 
        [&](size_t i, const char* p, size_t l) {
            counterexample[i] = std::string(p, p + l);
        }
    );
}

