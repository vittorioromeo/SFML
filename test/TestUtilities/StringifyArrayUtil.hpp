#pragma once

#include <doctest/parts/doctest_fwd.h>

#include <array>

namespace doctest
{

template <typename T, std::size_t N>
struct StringMaker<std::array<T, N>>
{
    static doctest::String convert(const std::array<T, N>&)
    {
        return ""; // TODO P2:
    }
};

} // namespace doctest
