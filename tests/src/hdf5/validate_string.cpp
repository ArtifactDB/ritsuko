#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/validate_string.hpp"
#include "utils.h"
#include <numeric>
#include <string>

TEST(Hdf5ValidateString, FixedScalar) {
    const char* path = "TEST-validate-string.h5";

    size_t strlen = 5;
    std::vector<char> buffer(strlen, 'a');

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace;
        H5::StrType stype(0, strlen);
        auto dhandle = handle.createDataSet("foobar", stype, dspace);
        dhandle.write(buffer.data(), stype);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openDataSet("foobar");
    ritsuko::hdf5::validate_scalar_string(dhandle);
}

TEST(Hdf5ValidateString, Fixed1dimensional) {
    const char* path = "TEST-validate-string.h5";

    // Doesn't really matter if it's compressed or not, as fixed-length strings are no-ops here.
    hsize_t dim = 77;
    size_t strlen = 3;
    std::vector<char> buffer(strlen * dim, 'a');

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace(1, &dim);
        H5::StrType stype(0, strlen);
        auto dhandle = handle.createDataSet("foobar", stype, dspace);
        dhandle.write(buffer.data(), stype);
    }

    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        ritsuko::hdf5::validate_1d_strings(dhandle, dim);
    }
}

TEST(Hdf5ValidateString, FixedNdimensional) {
    const char* path = "TEST-validate-string.h5";

    // Doesn't really matter if it's compressed or not, as fixed-length strings are no-ops here.
    std::vector<hsize_t> dims{ 77, 192 };
    std::vector<hsize_t> chunks{ 12, 15 };
    size_t strlen = 5;
    std::vector<char> buffer(strlen * dims[0] * dims[1], 'x');

    H5::DataSpace dspace(2, dims.data());
    H5::DSetCreatPropList cplist;
    cplist.setDeflate(6);
    cplist.setChunk(2, chunks.data());
    H5::StrType stype(0, strlen);

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("foobar", stype, dspace, cplist);
        dhandle.write(buffer.data(), stype);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        ritsuko::hdf5::validate_nd_strings(dhandle, dims);
    }
}

TEST(Hdf5ValidateString, VariableScalar) {
    const char* path = "TEST-validate-string.h5";

    H5::StrType stype(0, H5T_VARIABLE);

    // Empty dataset fails.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace;
        auto dhandle = handle.createDataSet("foobar", stype, dspace);
    }

    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        std::string msg;
        try {
            ritsuko::hdf5::validate_scalar_string(dhandle);
        } catch (std::exception& e) {
            msg = e.what();
        }
        EXPECT_THAT(msg, ::testing::HasSubstr("NULL pointer"));
    }

    // Passes once we fill it.
    {
        H5::H5File handle(path, H5F_ACC_RDWR);
        auto dhandle = handle.openDataSet("foobar");
        std::string okay = "okay";
        dhandle.write(okay, stype);
    }

    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        ritsuko::hdf5::validate_scalar_string(dhandle);
    }
}

class Hdf5ValidateStringTest : public ::testing::TestWithParam<bool> {};

TEST_P(Hdf5ValidateStringTest, Variable1dimensional) {
    const auto chunked = GetParam();
    const char* path = "TEST-validate-string.h5";

    hsize_t dim = 144;
    H5::DataSpace dspace(1, &dim);
    H5::DSetCreatPropList cplist;
    if (chunked) {
        hsize_t chunk_size = 19;
        cplist.setDeflate(6);
        cplist.setChunk(1, &chunk_size);
    }

    const char * placeholder = "akari";
    std::vector<const char*> ptrs(dim, placeholder);
    H5::StrType stype(0, H5T_VARIABLE);

    // Validation succeeds with valid strings.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("foobar", stype, dspace, cplist);
        dhandle.write(ptrs.data(), stype);
    }

    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        ritsuko::hdf5::validate_1d_strings(dhandle, dim);
    }

    // Now injecting a NULL at some key places and checking that the validator can find it.
    for (int scenario = 0; scenario < 3; ++scenario) {
        std::size_t loc; 
        if (scenario == 0) {
            loc = 0;
        } else if (scenario == 1) {
            loc = dim / 2;
        } else {
            loc = dim - 1;
        }

        ptrs[loc] = NULL;

        {
            H5::H5File handle(path, H5F_ACC_RDWR);
            auto dhandle = handle.openDataSet("foobar");
            dhandle.write(ptrs.data(), stype);
        }

        {
            H5::H5File handle(path, H5F_ACC_RDONLY);
            auto dhandle = handle.openDataSet("foobar");
            std::string msg;
            try {
                ritsuko::hdf5::validate_1d_strings(dhandle, dim);
            } catch (std::exception& e) {
                msg = e.what();
            }
            EXPECT_THAT(msg, ::testing::HasSubstr("NULL pointer"));
        }

        ptrs[loc] = placeholder;
    }
}

TEST_P(Hdf5ValidateStringTest, VariableNdimensional) {
    const auto chunked = GetParam();
    const char* path = "TEST-validate-string.h5";

    std::vector<hsize_t> dims{ 79, 175 };
    std::vector<hsize_t> chunks{ 10, 20 };
    H5::DSetCreatPropList cplist;
    if (chunked) {
        cplist.setDeflate(6);
        cplist.setChunk(2, chunks.data());
    }

    const char * placeholder = "superfoobar";
    H5::StrType stype(0, H5T_VARIABLE);
    hsize_t total_len = dims[0] * dims[1];
    std::vector<const char*> ptrs(total_len, placeholder);

    // Validation succeeds with valid strings.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        H5::DataSpace dspace(2, dims.data());
        auto dhandle = handle.createDataSet("foobar", stype, dspace, cplist);
        dhandle.write(ptrs.data(), stype);
    }

    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        ritsuko::hdf5::validate_nd_strings(dhandle, dims);
    }

    // Now injecting a NULL at some key places and checking that the validator can find it.
    for (int scenario = 0; scenario < 3; ++scenario) {
        std::size_t loc; 
        if (scenario == 0) {
            loc = 0;
        } else if (scenario == 1) {
            loc = total_len / 2;
        } else {
            loc = total_len - 1;
        }

        ptrs[loc] = NULL;

        {
            H5::H5File handle(path, H5F_ACC_RDWR);
            auto dhandle = handle.openDataSet("foobar");
            dhandle.write(ptrs.data(), stype);
        }

        {
            H5::H5File handle(path, H5F_ACC_RDONLY);
            auto dhandle = handle.openDataSet("foobar");
            std::string msg;
            try {
                ritsuko::hdf5::validate_nd_strings(dhandle, dims);
            } catch (std::exception& e) {
                msg = e.what();
            }
            EXPECT_THAT(msg, ::testing::HasSubstr("NULL pointer"));
        }

        ptrs[loc] = placeholder;
    }
}

INSTANTIATE_TEST_SUITE_P(
   Hdf5ValidateString,
   Hdf5ValidateStringTest,
   ::testing::Values(false, true)
);
