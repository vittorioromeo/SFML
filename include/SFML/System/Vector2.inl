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
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp> // NOLINT(misc-header-include-cycle)

#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Math.hpp>
#include <SFML/Base/Traits/IsFloatingPoint.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::lengthSq() const
{
    return dot(*this);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::perpendicular() const
{
    return Vector2<T>(-y, x);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::dot(Vector2<T> rhs) const
{
    return x * rhs.x + y * rhs.y;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::cross(Vector2<T> rhs) const
{
    return x * rhs.y - y * rhs.x;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::cwiseMul(Vector2<T> rhs) const
{
    return Vector2<T>(x * rhs.x, y * rhs.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::cwiseDiv(Vector2<T> rhs) const
{
    SFML_BASE_ASSERT(rhs.x != 0 && "Vector2::cwiseDiv() cannot divide by 0 (x coordinate)");
    SFML_BASE_ASSERT(rhs.y != 0 && "Vector2::cwiseDiv() cannot divide by 0 (y coordinate)");

    return Vector2<T>(x / rhs.x, y / rhs.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator-(Vector2<T> right)
{
    return Vector2<T>(-right.x, -right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator+=(Vector2<T>& left, Vector2<T> right)
{
    left.x += right.x;
    left.y += right.y;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator-=(Vector2<T>& left, Vector2<T> right)
{
    left.x -= right.x;
    left.y -= right.y;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator+(Vector2<T> left, Vector2<T> right)
{
    return Vector2<T>(left.x + right.x, left.y + right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator-(Vector2<T> left, Vector2<T> right)
{
    return Vector2<T>(left.x - right.x, left.y - right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator*(Vector2<T> left, T right)
{
    return Vector2<T>(left.x * right, left.y * right);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator*(T left, Vector2<T> right)
{
    return Vector2<T>(right.x * left, right.y * left);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator*=(Vector2<T>& left, T right)
{
    left.x *= right;
    left.y *= right;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> operator/(Vector2<T> left, T right)
{
    SFML_BASE_ASSERT(right != 0 && "Vector2::operator/ cannot divide by 0");

    return Vector2<T>(left.x / right, left.y / right);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T>& operator/=(Vector2<T>& left, T right)
{
    SFML_BASE_ASSERT(right != 0 && "Vector2::operator/= cannot divide by 0");

    left.x /= right;
    left.y /= right;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator==(Vector2<T> left, Vector2<T> right)
{
    return (left.x == right.x) && (left.y == right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator!=(Vector2<T> left, Vector2<T> right)
{
    return (left.x != right.x) || (left.y != right.y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::normalized() const
{
    static_assert(base::isFloatingPoint<T>, "Vector2::normalized() is only supported for floating point types");

    SFML_BASE_ASSERT(*this != Vector2<T>() && "Vector2::normalized() cannot normalize a zero vector");

    return (*this) / length();
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Angle Vector2<T>::angleTo(Vector2<T> rhs) const
{
    static_assert(base::isFloatingPoint<T>, "Vector2::angleTo() is only supported for floating point types");

    SFML_BASE_ASSERT(*this != Vector2<T>() && "Vector2::angleTo() cannot calculate angle from a zero vector");
    SFML_BASE_ASSERT(rhs != Vector2<T>() && "Vector2::angleTo() cannot calculate angle to a zero vector");

    return radians(static_cast<float>(base::atan2(cross(rhs), dot(rhs))));
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Angle Vector2<T>::angle() const
{
    static_assert(base::isFloatingPoint<T>, "Vector2::angle() is only supported for floating point types");

    SFML_BASE_ASSERT(*this != Vector2<T>() && "Vector2::angle() cannot calculate angle from a zero vector");

    return radians(static_cast<float>(base::atan2(y, x)));
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::rotatedBy(Angle phi) const
{
    static_assert(base::isFloatingPoint<T>, "Vector2::rotatedBy() is only supported for floating point types");

    // No zero vector assert, because rotating a zero vector is well-defined (yields always itself)
    const T cos = base::cos(static_cast<T>(phi.asRadians()));
    const T sin = base::sin(static_cast<T>(phi.asRadians()));

    // Don't manipulate x and y separately, otherwise they're overwritten too early
    return Vector2<T>(cos * x - sin * y, sin * x + cos * y);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::movedTowards(T r, Angle phi) const
{
    static_assert(base::isFloatingPoint<T>, "Vector2::movedTowards() is only supported for floating point types");

    return *this + Vector2<T>::fromAngle(r, phi);
}

////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::projectedOnto(Vector2<T> axis) const
{
    static_assert(base::isFloatingPoint<T>, "Vector2::projectedOnto() is only supported for floating point types");

    SFML_BASE_ASSERT(axis != Vector2<T>() && "Vector2::projectedOnto() cannot project onto a zero vector");
    return dot(axis) / axis.lengthSq() * axis;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector2<T> Vector2<T>::fromAngle(T r, Angle phi)
{
    static_assert(base::isFloatingPoint<T>, "Vector2::Vector2(T, Angle) is only supported for floating point types");
    return {r * static_cast<T>(base::cos(phi.asRadians())), r * static_cast<T>(base::sin(phi.asRadians()))};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector2<T>::length() const
{
    static_assert(base::isFloatingPoint<T>, "Vector2::length() is only supported for floating point types");

    // don't use std::hypot because of slow performance
    return base::sqrt(x * x + y * y);
}


////////////////////////////////////////////////////////////
template <typename T>
template <typename OtherVector2>
constexpr OtherVector2 Vector2<T>::to() const
{
    using ValueType = decltype(OtherVector2{}.x);
    return Vector2<ValueType>{static_cast<ValueType>(x), static_cast<ValueType>(y)};
}


////////////////////////////////////////////////////////////
// Static member data
////////////////////////////////////////////////////////////

template <typename T>
inline constexpr Vector2<T> Vector2<T>::UnitX(static_cast<T>(1), static_cast<T>(0));

template <typename T>
inline constexpr Vector2<T> Vector2<T>::UnitY(static_cast<T>(0), static_cast<T>(1));

template <typename T>
inline constexpr Vector2<T> Vector2<T>::Zero(static_cast<T>(0), static_cast<T>(0));

} // namespace sf
