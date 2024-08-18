#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Event;
} // namespace sf


////////////////////////////////////////////////////////////
/// \brief Event handling utilities
///
////////////////////////////////////////////////////////////
namespace sf::EventUtils
{
////////////////////////////////////////////////////////////
/// \brief Check if the event is `Event::Closed` or `Event::KeyPressed`
///        with escape key
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool isClosedOrEscapeKeyPressed(const Event& event);

} // namespace sf::EventUtils


////////////////////////////////////////////////////////////
/// \namespace sf::EventUtils
/// \ingroup window
///
/// Provides event handling utilities
///
/// \see sf::Event
///
////////////////////////////////////////////////////////////
