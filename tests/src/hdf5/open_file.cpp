#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "ritsuko/hdf5/open_file.hpp"
#include "utils.h"

static void expect_error(const char* name, std::string msg) {
    EXPECT_ANY_THROW({
        try {
            ritsuko::hdf5::open_file(name);
        } catch (std::exception& e) {
            EXPECT_THAT(e.what(), ::testing::HasSubstr(msg));
            throw;
        }
    });
}

TEST(Hdf5OpenFile, Basic) {
    const char* path = "TEST-open.h5";
    std::filesystem::remove(path);
    expect_error(path, "no file is present");

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
    }
    {
        ritsuko::hdf5::open_file(path);
    }
}
