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

#pragma once

#include <SFML/Base/IndexSequence.hpp>


#if __has_builtin(__integer_pack)

////////////////////////////////////////////////////////////
#define SFML_BASE_MAKE_INTEGER_SEQUENCE(N) ::sf::base::IndexSequence<__integer_pack(N)>


namespace sf::base
{
////////////////////////////////////////////////////////////
template <decltype(sizeof(int)) N>
using MakeIndexSequence = SFML_BASE_MAKE_INTEGER_SEQUENCE(N);

} // namespace sf::base

#elif __has_builtin(__make_integer_seq)

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T, T... Is>
struct MakeIndexSequenceHelper
{
    using type = IndexSequence<Is...>;
};

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <decltype(sizeof(int)) N>
using MakeIndexSequence = typename __make_integer_seq<priv::MakeIndexSequenceHelper, decltype(sizeof(int)), N>::type;

} // namespace sf::base


////////////////////////////////////////////////////////////
#define SFML_BASE_MAKE_INTEGER_SEQUENCE(N) ::sf::base::MakeIndexSequence<N>

#else

#include <utility>


namespace sf::base
{
////////////////////////////////////////////////////////////
template <decltype(sizeof(int)) N>
using MakeIndexSequence = std::make_index_sequence<N>;

} // namespace sf::base


////////////////////////////////////////////////////////////
#define SFML_BASE_MAKE_INTEGER_SEQUENCE(N) ::sf::base::MakeIndexSequence<N>

#endif
