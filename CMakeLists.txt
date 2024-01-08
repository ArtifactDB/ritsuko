cmake_minimum_required(VERSION 3.24)

project(ritsuko
    VERSION 0.5.0
    DESCRIPTION "Helper functions for ArtifactDB libraries"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(ritsuko INTERFACE)
add_library(artifactdb::ritsuko ALIAS ritsuko)

include(GNUInstallDirs)
target_include_directories(ritsuko INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/artifactdb_ritsuko>")

option(RITSUKO_FIND_HDF5 "Try to find and link to HDF5 for ritsuko." ON)
if(RITSUKO_FIND_HDF5)
    find_package(HDF5 COMPONENTS C CXX)
    if (HDF5_FOUND)
        target_link_libraries(ritsuko INTERFACE hdf5::hdf5 hdf5::hdf5_cpp)
    endif()
endif()

# Building the test-related machinery, if we are compiling this library directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(RITSUKO_TESTS "Build ritsuko's test suite." ON)
else()
    option(RITSUKO_TESTS "Build ritsuko's test suite." OFF)
endif()

if(RITSUKO_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()

# Setting up the installation commands.
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/artifactdb_ritsuko)

install(TARGETS ritsuko 
    EXPORT ritsukoTargets)

install(EXPORT ritsukoTargets
    FILE artifactdb_ritsukoTargets.cmake
    NAMESPACE artifactdb::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/artifactdb_ritsuko)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/artifactdb_ritsukoConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/artifactdb_ritsuko)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/artifactdb_ritsukoConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/artifactdb_ritsuko)
