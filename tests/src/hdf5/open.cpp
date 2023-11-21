#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "ritsuko/hdf5/open.hpp"
#include "utils.h"

TEST(Hdf5Open, File) {
    const char* path = "TEST-open.h5";
    std::filesystem::remove(path);
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::open_file(path);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr("no file is present"));
            throw;
        }
    });

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
    }
    {
        ritsuko::hdf5::open_file(path);
    }
}

static void expect_group_error(const H5::Group& handle, const char* name, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::open_group(handle, name);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5Open, Group) {
    const char* path = "TEST-group.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        handle.createGroup("foobar");
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        ritsuko::hdf5::open_group(handle, "foobar");
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        handle.createDataSet("foobar", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        expect_group_error(handle, "foobar", "expected a group");
        expect_group_error(handle, "blahblah", "expected a group");
    }
}

static void expect_dataset_error(const H5::Group& handle, const char* name, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::open_dataset(handle, name);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5Open, Dataset) {
    const char* path = "TEST-dataset.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        create_dataset(handle, "foobar", std::vector<int>{1,2,3}, H5::PredType::NATIVE_INT);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = ritsuko::hdf5::open_dataset(handle, "foobar");
        EXPECT_EQ(dhandle.getTypeClass(), H5T_INTEGER);
        EXPECT_EQ(dhandle.getSpace().getSimpleExtentNdims(), 1);
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        handle.createGroup("blahblah");
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        expect_dataset_error(handle, "foobar", "expected a dataset");
        expect_dataset_error(handle, "blahblah", "expected a dataset");
    }
}

static void expect_attribute_error(const H5::Group& handle, const char* name, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::open_attribute(handle, name);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5Open, ScalarAttribute) {
    std::string path = "TEST-attribute.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("whee");
        ghandle.createAttribute("okay", H5::PredType::NATIVE_INT, H5S_SCALAR);
        hsize_t ndim = 100;
        H5::DataSpace dspace(1, &ndim);
        ghandle.createAttribute("nooo", H5::PredType::NATIVE_INT, dspace);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto ghandle = handle.openGroup("whee");
    auto attr = ritsuko::hdf5::open_attribute(ghandle, "okay");
    EXPECT_EQ(attr.getTypeClass(), H5T_INTEGER);
    expect_attribute_error(ghandle, "missing", "expected an attribute");
    auto attr2 = ritsuko::hdf5::open_attribute(ghandle, "nooo");
    EXPECT_EQ(attr2.getTypeClass(), H5T_INTEGER);
}
