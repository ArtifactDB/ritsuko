# C++ utilities for parsing and validation

![Unit tests](https://github.com/ArtifactDB/ritsuko/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/ArtifactDB/ritsuko/actions/workflows/doxygenate.yaml/badge.svg)
[![codecov](https://codecov.io/gh/ArtifactDB/ritsuko/branch/master/graph/badge.svg?token=J3dxS3MtT1)](https://codecov.io/gh/ArtifactDB/ritsuko)

## Overview

**ritsuko** provides common utilities for parsing and validation throughout the [ArtifactDB](https://github.com/ArtifactDB) C++ codebase.
This is generally not intended for consumption by external developers, but they are nonetheless free to use it. 
Functionality includes some convenience wrappers for HDF5 parsing, date/time string checking functions, a definition of R's missing value, 
and more random stuff that is re-usable across different C++ libraries.

Check out the [reference documentation](https://artifactdb.github.io/ritsuko) for available functions.
Some usage examples are shown below.

## Non-HDF5 utilities

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

## HDF5 utilities

For the HDF5 utilities, we need to include a different header:

```cpp
#include "ritsuko/hdf5/hdf5.hpp"

// Opening a handle to a dataset.
H5::H5File handle("some.h5", H5F_ACC_RDONLY);
auto dhandle = handle.openDataSet("some_data");

// Reading some interesting bits and pieces.
auto len = ritsuko::hdf5::get_1d_length(dhandle, false);
auto tag = ritsuko::hdf5::open_and_load_scalar_string_attribute(dhandle, "tag");

// Iterating over a 1-dimensional dataset. 
ritsuko::hdf5::Stream1dNumericDataset<double> stream(
    &dhandle,
    len,
    /* buffer_size = */ 10000
);
for (hsize_t i = 0; i < len; ++i, stream.next()) {
    auto val = stream.get();
}

// Iterating over a N-dimensional dataset.
auto dims = ritsuko::hdf5::get_dimensions(dhandle, false);
auto blocks = ritsuko::hdf5::pick_nd_block_dimensions(
    dhandle.getCreatePlist(),
    dims,
    /* buffer_size = */ 10000
);

ritsuko::hdf5::IterateNdDataset iter(&dims, &blocks);
std::vector<double> buffer;
while (!iter.finished()) {
    buffer.resize(iter.size());
    dhandle.read(
        buffer.data(),
        H5::PredType::NATIVE_DOUBLE,
        iter.memory_space(),
        iter.file_space()
    );
    iter.next();
}
```

## Utilities for variable length string arrays

For **ritsuko**'s custom variable length string (VLS) arrays for HDF5, yet another header is involved:

```cpp
#include "ritsuko/hdf5/vls/vls.hpp"

// Opening handles to the VLS datasets.
H5::H5File handle("some.h5", H5F_ACC_RDONLY);
auto phandle = ritsuko::hdf5::vls::open_pointers(handle, "pointers", 64, 64);
auto hhandle = ritsuko::hdf5::vls::open_heap(handle, "heap");

ritsuko::hdf5::vls::Stream1dArray<uint64_t, uint64_t> stream(
    &dhandle,
    len,
    /* buffer_size = */ 1000
);
for (hsize_t i = 0; i < len; ++i, stream.next()) {
    auto val = stream.get();
}
```

## Building projects

### CMake with `FetchContent`

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

### CMake with `find_package()`

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

This library is named after [Ritsuko Akizuki](https://myanimelist.net/character/6170/Ritsuko_Akizuki), as befitting her important support role.

![Ritsuko GIF](https://media.tenor.com/I0ED_9E3vnwAAAAd/ritsuko-akizuki-idolmaster.gif)
