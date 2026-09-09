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

    src/format/is_date_time.cpp
    src/format/parse_version_string.cpp

    src/hdf5/exceeds_limit.cpp
    src/hdf5/is_utf8_string.cpp
    src/hdf5/Stream1dNumericDataset.cpp
    src/hdf5/Stream1dStringDataset.cpp
    src/hdf5/IterateChunks.cpp
    src/hdf5/validate_string.cpp
    src/hdf5/get_name.cpp
    src/hdf5/as_numeric_datatype.cpp

    src/cvls/Pointer.cpp
    src/cvls/Stream1dArray.cpp
    src/cvls/validate.cpp
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
