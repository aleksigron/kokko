#pragma once

#include <type_traits>

namespace kokko
{

template <typename T> struct IsZeroInitializable {
    static const constexpr bool Value = std::is_scalar<T>::value;
};

}
