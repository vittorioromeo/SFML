#pragma once

#include <SFML/Base/Optional.hpp>

#include <doctest/parts/doctest_fwd.h>

namespace doctest
{

template <typename T>
struct StringMaker<sf::base::Optional<T>>
{
    static doctest::String convert(const sf::base::Optional<T>&)
    {
        return ""; // TODO P2:
    }
};

} // namespace doctest
