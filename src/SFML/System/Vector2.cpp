////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Vector2.hpp>


////////////////////////////////////////////////////////////
// Explicit instantiation definitions
////////////////////////////////////////////////////////////

template class sf::Vector2<float>;
template class sf::Vector2<double>;
template class sf::Vector2<long double>;

#define SFML_INSTANTIATE_VECTOR2_BASIC_MEMBER_FUNCTIONS(type)  \
    template const sf::Vector2<type> sf::Vector2<type>::UnitX; \
    template const sf::Vector2<type> sf::Vector2<type>::UnitY; \
    template const sf::Vector2<type> sf::Vector2<type>::Zero;

#define SFML_INSTANTIATE_VECTOR2_INTEGRAL_MEMBER_FUNCTIONS(type)           \
    template type              sf::Vector2<type>::lengthSq() const;        \
    template sf::Vector2<type> sf::Vector2<type>::perpendicular() const;   \
    template type              sf::Vector2<type>::dot(Vector2) const;      \
    template type              sf::Vector2<type>::cross(Vector2) const;    \
    template sf::Vector2<type> sf::Vector2<type>::cwiseMul(Vector2) const; \
    template sf::Vector2<type> sf::Vector2<type>::cwiseDiv(Vector2) const;

SFML_INSTANTIATE_VECTOR2_BASIC_MEMBER_FUNCTIONS(bool)
SFML_INSTANTIATE_VECTOR2_BASIC_MEMBER_FUNCTIONS(int)
SFML_INSTANTIATE_VECTOR2_BASIC_MEMBER_FUNCTIONS(unsigned int)

SFML_INSTANTIATE_VECTOR2_INTEGRAL_MEMBER_FUNCTIONS(int)
SFML_INSTANTIATE_VECTOR2_INTEGRAL_MEMBER_FUNCTIONS(unsigned int)
