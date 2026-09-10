#ifndef RITSUKO_RITSUKO_HPP
#define RITSUKO_RITSUKO_HPP

#include "format/is_date_time.hpp"
#include "format/parse_version_string.hpp"

#include "hdf5/IterateChunks.hpp"
#include "hdf5/strnlen.hpp"
#include "hdf5/as_numeric_datatype.hpp"
#include "hdf5/mock_contiguous_chunks.hpp"
#include "hdf5/get_name.hpp"
#include "hdf5/ReclaimVlsMemory.hpp"
#include "hdf5/validate_string.hpp"
#include "hdf5/Stream1dStringDataset.hpp"
#include "hdf5/Stream1dNumericDataset.hpp"
#include "hdf5/exceeds_limit.hpp"
#include "hdf5/read_scalar_string.hpp"
#include "hdf5/is_utf8_string.hpp"

#include "cvls/Pointer.hpp"
#include "cvls/validate.hpp"
#include "cvls/Stream1dArray.hpp"

/**
 * @file ritsuko.hpp
 * @brief Umbrella header for **ritsuko**.
 */

/**
 * @namespace ritsuko
 * @brief Helper functions for ArtifactDB parsing and validation.
 */
namespace ritsuko {

/**
 * @namespace ritsuko::hdf5
 * @brief Utilities for reading and validating HDF5 files.
 */
namespace hdf5 {}

/**
 * @namespace ritsuko::cvls
 * @brief Utilities for reading and validating compressed VLS arrays.
 */
namespace cvls {}

}

#endif
