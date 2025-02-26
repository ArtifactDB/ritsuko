#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <cstdint>

#include "ritsuko/hdf5/vls/define_pointer_datatype.hpp"

#include "utils.h"

TEST(VlsDefinePointerDatatype, Basic) {
    {
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint64_t, uint64_t>();
        ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 64, 64);
    }

    // Works correctly with uneven types.
    {
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint64_t, uint32_t>();
        ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 64, 64);
    }

    // Works correctly with smaller, uneven types.
    {
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint8_t, uint32_t>();
        ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 32, 32);
    }
}

TEST(VlsDefinePointerDatatype, Usage) {
    size_t nlen = 10;
    std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint32_t> > data(nlen);
    for (size_t i = 0; i < nlen; ++i) {
        data[i].start = i * 1;
        data[i].size = i * 100;
    }

    // Creating a file.
    const std::string path = "TEST-vls-pointer.h5";
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint32_t>();
        create_vls_pointer_dataset(handle, "foo", data, dtype);
    }

    // Roundtrip with a higher type.
    {
        std::vector<ritsuko::hdf5::vls::Pointer<uint32_t, uint64_t> > roundtrip(nlen);
        auto out_type = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, uint64_t>();

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo");
        dhandle.read(roundtrip.data(), out_type);

        for (size_t i = 0; i < nlen; ++i) {
            EXPECT_EQ(roundtrip[i].start, data[i].start);
            EXPECT_EQ(roundtrip[i].size, data[i].size);
        }
    }

    // Roundtrip with a lower type.
    {
        std::vector<ritsuko::hdf5::vls::Pointer<uint64_t, uint16_t> > roundtrip(nlen);
        auto out_type = ritsuko::hdf5::vls::define_pointer_datatype<uint64_t, uint16_t>();

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foo");
        dhandle.read(roundtrip.data(), out_type);

        for (size_t i = 0; i < nlen; ++i) {
            EXPECT_EQ(roundtrip[i].start, data[i].start);
            EXPECT_EQ(roundtrip[i].size, data[i].size);
        }
    }
}

TEST(VlsValidatePointerDatatype, Failure) {
    {
        H5::CompType dtype(sizeof(uint64_t));
        dtype.insertMember("start", 0, ritsuko::hdf5::as_numeric_datatype<uint64_t>());
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 32, 32);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("two members"));
                throw;
            }
        });
    }

    {
        typedef ritsuko::hdf5::vls::Pointer<uint8_t, uint8_t> tmp;
        H5::CompType dtype(sizeof(tmp));
        dtype.insertMember("foo", HOFFSET(tmp, start), ritsuko::hdf5::as_numeric_datatype<uint8_t>());
        dtype.insertMember("bar", HOFFSET(tmp, size), ritsuko::hdf5::as_numeric_datatype<uint8_t>());
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 32, 32);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("start"));
                throw;
            }
        });
    }

    {
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<double, uint64_t>();
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 64, 64);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("integer"));
                throw;
            }
        });
    }

    {
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<int, uint64_t>();
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 64, 64);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("exceed"));
                throw;
            }
        });
    }

    {
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint64_t, uint64_t>();
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 32, 64);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("exceed"));
                throw;
            }
        });
    }

    {
        typedef ritsuko::hdf5::vls::Pointer<uint8_t, uint8_t> tmp;
        H5::CompType dtype(sizeof(tmp));
        dtype.insertMember("start", HOFFSET(tmp, start), ritsuko::hdf5::as_numeric_datatype<uint8_t>());
        dtype.insertMember("end", HOFFSET(tmp, size), ritsuko::hdf5::as_numeric_datatype<uint8_t>());
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 32, 32);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("size"));
                throw;
            }
        });
    }

    {
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint32_t, double>();
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 64, 64);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("integer"));
                throw;
            }
        });
    }

    {
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint64_t, int>();
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 64, 64);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("exceed"));
                throw;
            }
        });
    }

    {
        auto dtype = ritsuko::hdf5::vls::define_pointer_datatype<uint64_t, uint64_t>();
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::vls::validate_pointer_datatype(dtype, 64, 32);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("exceed"));
                throw;
            }
        });
    }
}
