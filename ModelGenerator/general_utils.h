#pragma once

#include <algorithm>

namespace utils {

template <typename T>
T EnforceInRange(T val, T from, T to)
{
    return std::max(from, std::min(val, to));
}

}  // namespace utils
