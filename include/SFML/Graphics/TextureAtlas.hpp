#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Texture.hpp"

#include "SFML/System/RectPacker.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Optional.hpp"

#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class Image;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief TODO P1: docs
///
////////////////////////////////////////////////////////////
class [[nodiscard]] SFML_GRAPHICS_API TextureAtlas
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit TextureAtlas(Texture&& atlasTexture);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Vector2f> add(const std::uint8_t* pixels, Vector2u size);
    [[nodiscard]] base::Optional<Vector2f> add(const Image& image);
    [[nodiscard]] base::Optional<Vector2f> add(const Texture& texture);

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Texture&       getTexture();
    [[nodiscard]] const Texture& getTexture() const;

    ////////////////////////////////////////////////////////////
    /// \brief TODO P1: docs
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] RectPacker&       getRectPacker();
    [[nodiscard]] const RectPacker& getRectPacker() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Texture    m_atlasTexture;
    RectPacker m_rectPacker;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::TextureAtlas
/// \ingroup graphics
///
/// TODO P1: docs
///
/// \see sf::Texture, sf::Image, sf::RenderTexture
///
////////////////////////////////////////////////////////////
