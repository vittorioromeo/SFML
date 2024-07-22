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

#include <SFML/System/Time.hpp>

#include <chrono>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Interoperability utilities between `sf::Time` and `<chrono>`
///
////////////////////////////////////////////////////////////
class TimeChronoUtil
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a `Time` from a `std::chrono::duration`
    ///
    ////////////////////////////////////////////////////////////
    template <typename Rep, typename Period>
    [[nodiscard]] static constexpr Time fromDuration(const std::chrono::duration<Rep, Period>& duration);

    ////////////////////////////////////////////////////////////
    /// \brief Convert `time` to a `std::chrono::microseconds` duration
    ///
    /// \return Time in microseconds
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static constexpr std::chrono::microseconds toDuration(Time time);

    ////////////////////////////////////////////////////////////
    /// \brief Convert `time` to a custom `std::chrono::duration`
    ///
    /// \return Duration in microseconds
    ///
    ////////////////////////////////////////////////////////////
    template <typename Rep, typename Period>
    [[nodiscard]] static constexpr std::chrono::duration<Rep, Period> toCustomDuration(Time time);
};

} // namespace sf

#include <SFML/System/TimeChronoUtil.inl>


////////////////////////////////////////////////////////////
/// \class sf::TimeChronoUtil
/// \ingroup system
///
/// Interoperability utilities between `sf::Time` and `<chrono>`
///
/// \see sf::Time, sf::Clock
///
////////////////////////////////////////////////////////////
