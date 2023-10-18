# C++ utilities for parsing and validation

![Unit tests](https://github.com/ArtifactDB/ritsuko/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/ArtifactDB/ritsuko/actions/workflows/doxygenate.yaml/badge.svg)
[![codecov](https://codecov.io/gh/ArtifactDB/ritsuko/branch/master/graph/badge.svg?token=J3dxS3MtT1)](https://codecov.io/gh/ArtifactDB/ritsuko)

## Overview

**ritsuko** provides common utilities for parsing and validation throughout the [ArtifactDB](https://github.com/ArtifactDB) C++ codebase.
This is generally not intended for consumption by external developers, but they are nonetheless free to use it. 
Functionality includes some convenience wrappers for HDF5 parsing, date/time string checking functions, a definition of R's missing value, 
and more random stuff that is re-usable across different C++ libraries.

## Quick start

For the non-HDF5-dependent utilities, we can just pull in **ritsuko**'s main header:

```cpp
#include "ritsuko/ritsuko.hpp"

// Get R's missing double-precision value.
auto rmissing = ritsuko::r_missing_value();

// Is a string formatted as a YYYY-MM-DD date?
std::string potential_date = "2031-11-21"
if (ritsuko::is_date(potential_date.c_str(), potential_date.size())) {
    // ...
}

// Is a string formatted as an RFC3339-compliant timestamp?
std::string potential_time = "2031-11-21T11:49:21.34+09:00"
if (ritsuko::is_rfc3339(potential_time.c_str(), potential_time.size())) {
    // ...  
}
```

For HDF5, the header is a little different:

```cpp
#include "ritsuko/hdf5/hdf5.hpp"

// Opening a handle to a dataset.
H5::H5File handle("some.h5", H5F_ACC_RDONLY);
auto dhandle = handle.openDataSet("some_data");

// Reading some interesting bits and pieces.
auto len = ritsuko::hdf5::get_1d_length(dhandle, "some_data");
auto tag = ritsuko::hdf5::load_scalar_string_attribute(dhandle, "tag", "some_data");

// Iterating over the dataset by block.
auto block_size = ritsuko::hdf5::pick_1d_block_size(dhandle.getCreatePlist(), len);
std::vector<double> buffer(len);
ritsuko::hdf5::iterate_1d_blocks(block_size, len, 
    [&](
        hsize_t start, 
        hsize_t len, 
        const H5::DataSpace& mspace, 
        const H5::DataSpace& dspace
    ) {
        dhandle.read(buffer.data() + start, H5::PredType::NATIVE_DOUBLE, mspace, dspace);
    }
);
```

Also see the [reference documentation](https://artifactdb.github.io/ritsuko) for more details.

### Building projects

#### CMake with `FetchContent`

If you're using CMake, you just need to add something like this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  ritsuko 
  GIT_REPOSITORY https://github.com/ArtifactDB/ritsuko
  GIT_TAG master # or any version of interest
)

FetchContent_MakeAvailable(ritsuko)
```

Then you can link to **ritsuko** to make the headers available during compilation:

```cmake
# For executables:
target_link_libraries(myexe ritsuko)

# For libaries
target_link_libraries(mylib INTERFACE ritsuko)
```

#### CMake with `find_package()`

You can install the library by cloning a suitable version of this repository and running the following commands:

```sh
mkdir build && cd build
cmake .. -DRITSUKO_TESTS=OFF
cmake --build . --target install
```

Then you can use `find_package()` as usual:

```cmake
find_package(artifactdb_ritsuko CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE artifactdb::ritsuko)
```

## Further remarks

This library is named after [Ritsuko Akizuki](https://myanimelist.net/character/6170/Ritsuko_Akizuki), 
as befitting her important role in supporting the other idols in the series.

![Ritsuko GIF](https://media.tenor.com/I0ED_9E3vnwAAAAd/ritsuko-akizuki-idolmaster.gif)
