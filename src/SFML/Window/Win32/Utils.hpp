#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>

using DWORD = unsigned long;

namespace sf::priv
{
////////////////////////////////////////////////////////////
std::string getErrorString(DWORD error);

} // namespace sf::priv
