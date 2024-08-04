#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Keyboard.hpp> // sf::Keyboard::Key

#include <X11/X.h> // KeySym

namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Convert X11 KeySym to sf::Keyboard::Key
///
/// \param symbol X11 KeySym
///
/// \return The corresponding sf::Keyboard::Key
///
////////////////////////////////////////////////////////////
Keyboard::Key keySymToKey(KeySym symbol);


////////////////////////////////////////////////////////////
/// \brief Convert sf::Keyboard::Key to X11 KeySym
///
/// \param key X11 sf::Keyboard::Key
///
/// \return The corresponding X11 KeySym
///
////////////////////////////////////////////////////////////
KeySym keyToKeySym(Keyboard::Key key);

} // namespace sf::priv
