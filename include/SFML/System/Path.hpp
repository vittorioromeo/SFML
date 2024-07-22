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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/Base/InPlacePImpl.hpp>

#include <iosfwd>


namespace sf
{
////////////////////////////////////////////////////////////
class [[nodiscard]] Path
{
public:
    using value_type = wchar_t;
    // using string_type = std::basic_string<value_type>;

    [[nodiscard]] static Path tempDirectoryPath();

    [[nodiscard]] explicit Path();

    template <typename T>
    [[nodiscard]] Path(const T& source);

    template <typename T>
    [[nodiscard]] Path(const T* source);

    // [[nodiscard]] Path(const value_type* str);
    // [[nodiscard]] Path(const string_type& str);
    // [[nodiscard]] Path(const char* str);
    // [[nodiscard]] Path(const std::string& str);

    ~Path();

    Path(const Path&);
    Path& operator=(const Path&);

    Path(Path&&) noexcept;
    Path& operator=(Path&&) noexcept;

    [[nodiscard]] Path filename() const;
    [[nodiscard]] Path extension() const;
    [[nodiscard]] Path absolute() const;

    [[nodiscard]] const value_type* c_str() const; // TODO P1: support other platforms
    // [[nodiscard]] std::string       string() const;

    // [[nodiscard]] operator std::string() const;

    template <typename T>
    T to() const;

    [[nodiscard]] bool remove() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] bool exists() const;

    Path& operator/=(const Path& rhs);

    friend Path          operator/(const Path& lhs, const Path& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Path& path);

    template <typename T>
    bool operator==(const T* str) const;

    template <typename T>
    bool operator!=(const T* str) const;

    template <typename T>
    bool operator==(const T& str) const;

    template <typename T>
    bool operator!=(const T& str) const;

private:
    [[nodiscard]] Path(int, const void* fsPath);

    struct Impl;
    base::InPlacePImpl<Impl, 64> m_impl;
};


} // namespace sf
