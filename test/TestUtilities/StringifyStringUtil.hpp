#pragma once

#include <doctest/parts/doctest_fwd.h>

#include <string>

namespace doctest
{

template <typename T>
struct StringMaker<std::basic_string<T>>
{
    static doctest::String convert(const std::basic_string<T>&)
    {
        return ""; // TODO P2:
    }
};

} // namespace doctest
