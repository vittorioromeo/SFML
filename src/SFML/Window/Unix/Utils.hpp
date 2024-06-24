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
#include <SFML/System/UniquePtr.hpp>

#include <X11/Xlib.h>

namespace sf::priv
{
template <typename T>
struct RemoveArrayExtentsImpl
{
    using type = T;
};

template <typename T>
struct RemoveArrayExtentsImpl<T[]>
{
    using type = T;
};

template <typename T>
using RemoveArrayExtents = typename RemoveArrayExtentsImpl<T>::type;

////////////////////////////////////////////////////////////
/// \brief Class template for freeing X11 pointers
///
/// Specialized elsewhere for types that are freed through
/// other means than XFree(). XFree() is the most common use
/// case though so it is the default.
///
////////////////////////////////////////////////////////////
template <typename T>
struct XDeleter
{
    void operator()(T* data) const
    {
        XFree(data); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    }
};

////////////////////////////////////////////////////////////
/// \brief Class template for wrapping owning raw pointers from X11
///
////////////////////////////////////////////////////////////
template <typename T>
using X11Ptr = UniquePtr<RemoveArrayExtents<T>, XDeleter<RemoveArrayExtents<T>>>;
} // namespace sf::priv
