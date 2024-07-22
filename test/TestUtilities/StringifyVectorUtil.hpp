#pragma once

#include <doctest/parts/doctest_fwd.h>

#include <vector>

namespace doctest
{

template <typename T>
struct StringMaker<std::vector<T>>
{
    static doctest::String convert(const std::vector<T>&)
    {
        return ""; // TODO P2:
    }
};

} // namespace doctest
