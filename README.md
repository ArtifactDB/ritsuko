# C++ utilities for parsing and validation

![Unit tests](https://github.com/ArtifactDB/ritsuko/actions/workflows/run-tests.yaml/badge.svg)
![Documentation](https://github.com/ArtifactDB/ritsuko/actions/workflows/doxygenate.yaml/badge.svg)
[![codecov](https://codecov.io/gh/ArtifactDB/ritsuko/branch/master/graph/badge.svg?token=J3dxS3MtT1)](https://codecov.io/gh/ArtifactDB/ritsuko)

## Overview

**ritsuko** provides common utilities for parsing and validation throughout the [ArtifactDB](https://github.com/ArtifactDB) C++ codebase.
This is generally not intended for consumption by external developers, but they are nonetheless free to use it. 
Functionality includes some convenience functions for HDF5 parsing, validation of compressed VLS arrays, and date/time string checking functions.jV

Check out the [reference documentation](https://artifactdb.github.io/ritsuko) for available functions.
In addition, the following documents contain some explanations on the design decisions used in ArtifactDB:

- [HDF5 internal organization](docs/topics/internal-organization.md)
- [Datatype constraints](docs/topics/datatype-constraints.md)
- [Missing value placeholders](docs/topics/missing-placeholder.md)
- [Compressed variable-length strings](docs/topics/compressed-vls.md)

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

This library is named after [Ritsuko Akizuki](https://myanimelist.net/character/6170/Ritsuko_Akizuki). 

![Ritsuko GIF](https://media.tenor.com/I0ED_9E3vnwAAAAd/ritsuko-akizuki-idolmaster.gif)
