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
#include <SFML/System/Rect.hpp> // NOLINT(misc-header-include-cycle)

#include <SFML/Base/Algorithm.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
constexpr bool Rect<T>::contains(Vector2<T> point) const
{
    // Rectangles with negative dimensions are allowed, so we must handle them correctly

    // Compute the real min and max of the rectangle on both axes
    const T minX = base::min(position.x, static_cast<T>(position.x + size.x));
    const T maxX = base::max(position.x, static_cast<T>(position.x + size.x));
    const T minY = base::min(position.y, static_cast<T>(position.y + size.y));
    const T maxY = base::max(position.y, static_cast<T>(position.y + size.y));

    return (point.x >= minX) && (point.x < maxX) && (point.y >= minY) && (point.y < maxY);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Rect<T>::getCenter() const
{
    return position + size / T{2};
}


////////////////////////////////////////////////////////////
template <typename T>
template <typename OtherRect>
constexpr OtherRect Rect<T>::to() const
{
    using ValueType = decltype(OtherRect{}.position.x);
    return Rect<ValueType>{position.template to<Vector2<ValueType>>(), size.template to<Vector2<ValueType>>()};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator==(const Rect<T>& lhs, const Rect<T>& rhs)
{
    return (lhs.position == rhs.position) && (lhs.size == rhs.size);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator!=(const Rect<T>& lhs, const Rect<T>& rhs)
{
    return !(lhs == rhs);
}

} // namespace sf
