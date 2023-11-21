include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/c9461a9b55ba954df0489bab6420eb297bed846b.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Avoid installing GoogleTest when installing this project.
option(INSTALL_GTEST "Enable installation of googletest." OFF)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Main test executable.
add_executable(
    libtest 
    src/r_missing_value.cpp
    src/is_date_time.cpp
    src/choose_missing_placeholder.cpp
    src/parse_version_string.cpp
    src/hdf5/exceeds_limit.cpp
    src/hdf5/load_scalar_string_attribute.cpp

    src/hdf5/get_1d_length.cpp
    src/hdf5/pick_1d_block_size.cpp

    src/hdf5/Numeric1dDatasetStream.cpp
    src/hdf5/iterate_1d_blocks.cpp
    src/hdf5/load_1d_string_dataset.cpp

    src/hdf5/open.cpp

    src/hdf5/get_missing_placeholder_attribute.cpp
    src/hdf5/get_name.cpp
    src/hdf5/as_numeric_datatype.cpp
)

target_link_libraries(
    libtest
    gtest_main
    gmock_main 
    ritsuko
)

target_compile_options(libtest PRIVATE -Wall -Wextra -Wpedantic -Werror)

set(CODE_COVERAGE OFF CACHE BOOL "Enable coverage testing")
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(libtest PRIVATE -O0 -g --coverage)
    target_link_options(libtest PRIVATE --coverage)
endif()

include(GoogleTest)
gtest_discover_tests(libtest)
