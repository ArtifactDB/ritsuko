#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ritsuko/hdf5/load_attribute.hpp"

TEST(Hdf5LoadAttribute, ScalarString) {
    const char* path = "TEST-scalar-attr.h5";

    // Variable.
    {
        const char* attr_name = "my_attr";
        std::string attr_val = "fooble";
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            auto dhandle = handle.createDataSet("INT8", H5::PredType::NATIVE_INT8, H5S_SCALAR);
            H5::StrType stype(0, H5T_VARIABLE);
            auto ahandle = dhandle.createAttribute(attr_name, stype, H5S_SCALAR);
            ahandle.write(stype, attr_val);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("INT8");
        auto ahandle = dhandle.openAttribute(attr_name);
        EXPECT_EQ(ritsuko::hdf5::load_scalar_string_attribute(ahandle), attr_val);
    }

    // Fixed.
    {
        const char* attr_name = "my_attr2";
        std::string attr_val = "supercagilfrag";
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            auto ghandle = handle.createGroup("UINT8");
            H5::StrType stype(0, 20);
            auto ahandle = ghandle.createAttribute(attr_name, stype, H5S_SCALAR);
            ahandle.write(stype, attr_val);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto ghandle = handle.openGroup("UINT8");
        auto ahandle = ghandle.openAttribute(attr_name);
        EXPECT_EQ(ritsuko::hdf5::load_scalar_string_attribute(ahandle), attr_val);
    }

    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            auto ghandle = handle.createGroup("whee");
            ghandle.createAttribute("stuff", H5::StrType(0, H5T_VARIABLE), H5S_SCALAR);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openGroup("whee");
        auto ahandle = dhandle.openAttribute("stuff");
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::load_scalar_string_attribute(ahandle);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("NULL pointer"));
                throw;
            }
        });
    }
 }

TEST(Hdf5LoadAttribute, Fixed1d) {
    const char* path = "TEST-1d-attr.h5";

    std::vector<std::string> example(19);
    for (size_t i = 0; i < example.size(); ++i) {
        example[i] = std::to_string(i);
    }

    {
        size_t maxlen = 1;
        for (const auto& v : example) {
            if (v.size() > maxlen) {
                maxlen = v.size();
            }
        }

        std::vector<char> buffer(maxlen * example.size());
        for (size_t v = 0; v < example.size(); ++v) {
            const auto& current = example[v];
            std::copy(current.begin(), current.end(), buffer.data() + v * maxlen);
        }

        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("whee");
        H5::StrType stype(0, maxlen);
        hsize_t dim = example.size();
        H5::DataSpace dspace(1, &dim);
        auto ahandle = ghandle.createAttribute("foo", stype, dspace);
        ahandle.write(stype, buffer.data());
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openGroup("whee");
    auto ahandle = dhandle.openAttribute("foo");
    auto counterexample = ritsuko::hdf5::load_1d_string_attribute(ahandle);

    EXPECT_EQ(example, counterexample);
}

TEST(Hdf5LoadAttribute, Variable1d) {
    const char* path = "TEST-1d-attr.h5";

    {
        std::vector<std::string> example(19);
        for (size_t i = 0; i < example.size(); ++i) {
            example[i] = std::to_string(i);
        }

        {
            std::vector<const char*> ptrs;
            ptrs.reserve(example.size());
            for (const auto& v : example) {
                ptrs.push_back(v.c_str());
            }

            H5::H5File handle(path, H5F_ACC_TRUNC);
            auto ghandle = handle.createGroup("whee");
            H5::StrType stype(0, H5T_VARIABLE);
            hsize_t dim = example.size();
            H5::DataSpace dspace(1, &dim);
            auto ahandle = ghandle.createAttribute("foo", stype, dspace);
            ahandle.write(stype, ptrs.data());
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openGroup("whee");
        auto ahandle = dhandle.openAttribute("foo");
        auto counterexample = ritsuko::hdf5::load_1d_string_attribute(ahandle);

        EXPECT_EQ(example, counterexample);
    }

    {
        {
            H5::H5File handle(path, H5F_ACC_TRUNC);
            auto ghandle = handle.createGroup("whee");
            hsize_t len = 10;
            H5::DataSpace dspace(1, &len);
            ghandle.createAttribute("stuff", H5::StrType(0, H5T_VARIABLE), dspace);
        }

        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openGroup("whee");
        auto ahandle = dhandle.openAttribute("stuff");
        EXPECT_ANY_THROW({
            try {
                ritsuko::hdf5::load_1d_string_attribute(ahandle);
            } catch (std::exception& e) {
                EXPECT_THAT(e.what(), ::testing::HasSubstr("NULL pointer"));
                throw;
            }
        });
    }
}

TEST(Hdf5LoadAttribute, ScalarNumeric) {
    const char* path = "TEST-scalar-attr.h5";

    const char* attr_name = "my_attr";
    int attr_val = 32;
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("whee");
        auto ahandle = ghandle.createAttribute(attr_name, H5::PredType::NATIVE_INT8, H5S_SCALAR);
        ahandle.write(H5::PredType::NATIVE_INT, &attr_val);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openGroup("whee");
    auto ahandle = dhandle.openAttribute(attr_name);
    EXPECT_EQ(ritsuko::hdf5::load_scalar_numeric_attribute<int32_t>(ahandle), attr_val);
}

TEST(Hdf5LoadAttribute, Numeric1d) {
    const char* path = "TEST-1d-attr.h5";

    const char* attr_name = "my_attr";
    std::vector<double> attr_val { 1.4, 2.2, -1.0 };
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("whee");
        hsize_t dim = attr_val.size();
        H5::DataSpace dspace(1, &dim);
        auto ahandle = ghandle.createAttribute(attr_name, H5::PredType::NATIVE_DOUBLE, dspace);
        ahandle.write(H5::PredType::NATIVE_DOUBLE, attr_val.data());
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto dhandle = handle.openGroup("whee");
    auto ahandle = dhandle.openAttribute(attr_name);
    EXPECT_EQ(ritsuko::hdf5::load_1d_numeric_attribute<double>(ahandle), attr_val);
}
