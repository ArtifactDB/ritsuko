#ifndef RITSUKO_UTILS_HPP
#define RITSUKO_UTILS_HPP

#include <type_traits>

namespace ritsuko {

template<typename Input_>
using I = std::remove_cv_t<std::remove_reference_t<Input_> >;

}

#endif
