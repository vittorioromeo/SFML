#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/System/String.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Holds various information about a font
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API FontInfo
{
    String family; //!< The font family
};

} // namespace sf
