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
#include <SFML/System/Vector3.hpp> // NOLINTNEXTLINE(misc-header-include-cycle)

#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Math.hpp>
#include <SFML/Base/Traits/IsFloatingPoint.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> Vector3<T>::normalized() const
{
    static_assert(base::isFloatingPoint<T>, "Vector3::normalized() is only supported for floating point types");

    SFML_BASE_ASSERT(*this != Vector3<T>() && "Vector3::normalized() cannot normalize a zero vector");
    return (*this) / length();
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector3<T>::length() const
{
    static_assert(base::isFloatingPoint<T>, "Vector3::length() is only supported for floating point types");

    // don't use std::hypot because of slow performance
    return base::sqrt(x * x + y * y + z * z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector3<T>::lengthSq() const
{
    return dot(*this);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr T Vector3<T>::dot(const Vector3<T>& rhs) const
{
    return x * rhs.x + y * rhs.y + z * rhs.z;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> Vector3<T>::cross(const Vector3<T>& rhs) const
{
    return Vector3<T>((y * rhs.z) - (z * rhs.y), (z * rhs.x) - (x * rhs.z), (x * rhs.y) - (y * rhs.x));
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> Vector3<T>::cwiseMul(const Vector3<T>& rhs) const
{
    return Vector3<T>(x * rhs.x, y * rhs.y, z * rhs.z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> Vector3<T>::cwiseDiv(const Vector3<T>& rhs) const
{
    SFML_BASE_ASSERT(rhs.x != 0 && "Vector3::cwiseDiv() cannot divide by 0 (x coordinate)");
    SFML_BASE_ASSERT(rhs.y != 0 && "Vector3::cwiseDiv() cannot divide by 0 (y coordinate)");
    SFML_BASE_ASSERT(rhs.z != 0 && "Vector3::cwiseDiv() cannot divide by 0 (z coordinate)");

    return Vector3<T>(x / rhs.x, y / rhs.y, z / rhs.z);
}


////////////////////////////////////////////////////////////
template <typename T>
template <typename OtherVector3>
constexpr OtherVector3 Vector3<T>::to() const
{
    using ValueType = decltype(OtherVector3{}.x);
    return Vector3<ValueType>{static_cast<ValueType>(x), static_cast<ValueType>(y), static_cast<ValueType>(z)};
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator-(const Vector3<T>& left)
{
    return Vector3<T>(-left.x, -left.y, -left.z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T>& operator+=(Vector3<T>& left, const Vector3<T>& right)
{
    left.x += right.x;
    left.y += right.y;
    left.z += right.z;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T>& operator-=(Vector3<T>& left, const Vector3<T>& right)
{
    left.x -= right.x;
    left.y -= right.y;
    left.z -= right.z;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator+(const Vector3<T>& left, const Vector3<T>& right)
{
    return Vector3<T>(left.x + right.x, left.y + right.y, left.z + right.z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator-(const Vector3<T>& left, const Vector3<T>& right)
{
    return Vector3<T>(left.x - right.x, left.y - right.y, left.z - right.z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator*(const Vector3<T>& left, T right)
{
    return Vector3<T>(left.x * right, left.y * right, left.z * right);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator*(T left, const Vector3<T>& right)
{
    return Vector3<T>(right.x * left, right.y * left, right.z * left);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T>& operator*=(Vector3<T>& left, T right)
{
    left.x *= right;
    left.y *= right;
    left.z *= right;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T> operator/(const Vector3<T>& left, T right)
{
    SFML_BASE_ASSERT(right != 0 && "Vector3::operator/ cannot divide by 0");

    return Vector3<T>(left.x / right, left.y / right, left.z / right);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr Vector3<T>& operator/=(Vector3<T>& left, T right)
{
    SFML_BASE_ASSERT(right != 0 && "Vector3::operator/= cannot divide by 0");

    left.x /= right;
    left.y /= right;
    left.z /= right;

    return left;
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator==(const Vector3<T>& left, const Vector3<T>& right)
{
    return (left.x == right.x) && (left.y == right.y) && (left.z == right.z);
}


////////////////////////////////////////////////////////////
template <typename T>
constexpr bool operator!=(const Vector3<T>& left, const Vector3<T>& right)
{
    return (left.x != right.x) || (left.y != right.y) || (left.z != right.z);
}

} // namespace sf
