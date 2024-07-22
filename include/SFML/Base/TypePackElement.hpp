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


#if __has_builtin(__type_pack_element)

////////////////////////////////////////////////////////////
#define SFML_BASE_TYPE_PACK_ELEMENT(N, ...) __type_pack_element<N, __VA_ARGS__>


namespace sf::base
{
////////////////////////////////////////////////////////////
template <decltype(sizeof(int)) N, typename... Ts>
using typePackElement = SFML_BASE_TYPE_PACK_ELEMENT(N, Ts...);

} // namespace sf::base

#else

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <typename T>
struct TypeWrapper
{
    using type = T;
};


////////////////////////////////////////////////////////////
template <decltype(sizeof(int)) N,
          typename T0 = void,
          typename T1 = void,
          typename T2 = void,
          typename T3 = void,
          typename T4 = void,
          typename T5 = void,
          typename T6 = void,
          typename T7 = void,
          typename T8 = void,
          typename T9 = void,
          typename... Ts>
[[nodiscard, gnu::always_inline]] consteval auto typePackElementImpl() noexcept
{
    // clang-format off
    if constexpr(N == 0)      { return TypeWrapper<T0>{}; }
    else if constexpr(N == 1) { return TypeWrapper<T1>{}; }
    else if constexpr(N == 2) { return TypeWrapper<T2>{}; }
    else if constexpr(N == 3) { return TypeWrapper<T3>{}; }
    else if constexpr(N == 4) { return TypeWrapper<T4>{}; }
    else if constexpr(N == 5) { return TypeWrapper<T5>{}; }
    else if constexpr(N == 6) { return TypeWrapper<T6>{}; }
    else if constexpr(N == 7) { return TypeWrapper<T7>{}; }
    else if constexpr(N == 8) { return TypeWrapper<T8>{}; }
    else if constexpr(N == 9) { return TypeWrapper<T9>{}; }
    else                      { return typePackElementImpl<N - 10, Ts...>(); }
    // clang-format on
}

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <SizeT N, typename... Ts>
using typePackElement = typename decltype(typePackElementImpl<N, Ts...>())::type;


////////////////////////////////////////////////////////////
#define SFML_BASE_TYPE_PACK_ELEMENT(N, ...) ::sf::base::typePackElement<N, __VA_ARGS__>

} // namespace sf::base

#endif
