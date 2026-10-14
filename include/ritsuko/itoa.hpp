#ifndef RITSUKO_ITOA_HPP
#define RITSUKO_ITOA_HPP

#include <string>
#include <algorithm>
#include <type_traits>

/**
 * @file itoa.hpp
 * @brief Convert an integer to a string.
 */

namespace ritsuko {

/**
 * This implementation is provided as a locale-independent alternative to `std::to_string()`.
 * It also avoids ambiguity in overload selection if the integer is not an alias of the types accepted by the `std::to_string()` overloads.
 *
 * @tparam Integer_ Integer type.
 *
 * @param x Integer value.
 * This is usually zero or positive in most **ritsuko**-related applications, but negative values are also allowed.
 * @param output Buffer to which the string representation of `x` is appended.
 */
template<typename Integer_>
void itoa(Integer_ x, std::string& output) {
    static_assert(std::is_integral<Integer_>::value);
    if (x > 0) {
        const auto sofar = output.size();
        while (x) {
            output += static_cast<char>('0' + x % 10);
            x /= 10;
        }
        std::reverse(output.begin() + sofar, output.end());
    } else if (x == 0) {
        output += '0';
    } else {
        output += '-';
        const auto sofar = output.size();
        while (x) {
            output += static_cast<char>('0' - (x % 10));
            x /= 10;
        }
        std::reverse(output.begin() + sofar, output.end());
    }
}

/**
 * Overload of `itoa()` that returns a new string.
 *
 * @tparam Integer_ Integer type.
 *
 * @param x Integer value, see comments in `itoa()`.
 *
 * @return String representation of `x`.
 */
template<typename Integer_>
std::string itoa(Integer_ x) {
    std::string output;
    itoa(x, output);
    return output;
}

}

#endif
