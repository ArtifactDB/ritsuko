#ifndef RITSUKO_R_MISSING_HPP
#define RITSUKO_R_MISSING_HPP

#include <cstdint>

namespace ritsuko {

/**
 * @return NaN with a payload of 1954, equivalent to the missing value in R.
 */
inline double r_missing() {
    uint32_t tmp_value = 1;
    auto tmp_ptr = reinterpret_cast<unsigned char*>(&tmp_value);

    // Mimic R's generation of these values, but we can't use type punning as
    // this is not legal in C++, and we don't have bit_cast yet.
    double missing_value = 0;
    auto missing_ptr = reinterpret_cast<unsigned char*>(&missing_value);

    int step = 1;
    if (tmp_ptr[0] == 1) { // little-endian. 
        missing_ptr += sizeof(double) - 1;
        step = -1;
    }

    *missing_ptr = 0x7f;
    *(missing_ptr += step) = 0xf0;
    *(missing_ptr += step) = 0x00;
    *(missing_ptr += step) = 0x00;
    *(missing_ptr += step) = 0x00;
    *(missing_ptr += step) = 0x00;
    *(missing_ptr += step) = 0x07;
    *(missing_ptr += step) = 0xa2;

    return missing_value;
}

}

#endif
