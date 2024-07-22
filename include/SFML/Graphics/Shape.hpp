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
#include <SFML/Graphics/Export.hpp>

#include <SFML/Graphics/Transformable.hpp>

#include <SFML/System/Rect.hpp>
#include <SFML/System/Vector2.hpp>

#include <SFML/Base/InPlacePImpl.hpp>

#include <cstddef>


namespace sf
{
struct RenderStates;
class RenderTarget;
class Texture;
struct Color;

////////////////////////////////////////////////////////////
/// \brief Base class for textured shapes with outline
///
////////////////////////////////////////////////////////////
class SFML_GRAPHICS_API Shape : public Transformable
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    Shape();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Shape();

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Shape(const Shape&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Shape& operator=(const Shape&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Shape(Shape&&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Shape& operator=(Shape&&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Set the sub-rectangle of the texture that the shape will display
    ///
    /// The texture rect is useful when you don't want to display
    /// the whole texture, but rather a part of it.
    /// By default, the texture rect covers the entire texture.
    ///
    /// \param rect Rectangle defining the region of the texture to display
    ///
    /// \see getTextureRect
    ///
    ////////////////////////////////////////////////////////////
    void setTextureRect(const IntRect& rect);

    ////////////////////////////////////////////////////////////
    /// \brief Set the fill color of the shape
    ///
    /// This color is modulated (multiplied) with the shape's
    /// texture if any. It can be used to colorize the shape,
    /// or change its global opacity.
    /// You can use sf::Color::Transparent to make the inside of
    /// the shape transparent, and have the outline alone.
    /// By default, the shape's fill color is opaque white.
    ///
    /// \param color New color of the shape
    ///
    /// \see getFillColor, setOutlineColor
    ///
    ////////////////////////////////////////////////////////////
    void setFillColor(const Color& color);

    ////////////////////////////////////////////////////////////
    /// \brief Set the outline color of the shape
    ///
    /// By default, the shape's outline color is opaque white.
    ///
    /// \param color New outline color of the shape
    ///
    /// \see getOutlineColor, setFillColor
    ///
    ////////////////////////////////////////////////////////////
    void setOutlineColor(const Color& color);

    ////////////////////////////////////////////////////////////
    /// \brief Set the thickness of the shape's outline
    ///
    /// Note that negative values are allowed (so that the outline
    /// expands towards the center of the shape), and using zero
    /// disables the outline.
    /// By default, the outline thickness is 0.
    ///
    /// \param thickness New outline thickness
    ///
    /// \see getOutlineThickness
    ///
    ////////////////////////////////////////////////////////////
    void setOutlineThickness(float thickness);

    ////////////////////////////////////////////////////////////
    /// \brief Get the sub-rectangle of the texture displayed by the shape
    ///
    /// \return Texture rectangle of the shape
    ///
    /// \see setTextureRect
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const IntRect& getTextureRect() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the fill color of the shape
    ///
    /// \return Fill color of the shape
    ///
    /// \see setFillColor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Color& getFillColor() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the outline color of the shape
    ///
    /// \return Outline color of the shape
    ///
    /// \see setOutlineColor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Color& getOutlineColor() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the outline thickness of the shape
    ///
    /// \return Outline thickness of the shape
    ///
    /// \see setOutlineThickness
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] float getOutlineThickness() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the local bounding rectangle of the entity
    ///
    /// The returned rectangle is in local coordinates, which means
    /// that it ignores the transformations (translation, rotation,
    /// scale, ...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// entity in the entity's coordinate system.
    ///
    /// \return Local bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const FloatRect& getLocalBounds() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the global (non-minimal) bounding rectangle of the entity
    ///
    /// The returned rectangle is in global coordinates, which means
    /// that it takes into account the transformations (translation,
    /// rotation, scale, ...) that are applied to the entity.
    /// In other words, this function returns the bounds of the
    /// shape in the global 2D world's coordinate system.
    ///
    /// This function does not necessarily return the \a minimal
    /// bounding rectangle. It merely ensures that the returned
    /// rectangle covers all the vertices (but possibly more).
    /// This allows for a fast approximation of the bounds as a
    /// first check; you may want to use more precise checks
    /// on top of that.
    ///
    /// \return Global bounding rectangle of the entity
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] FloatRect getGlobalBounds() const;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Recompute the internal geometry of the shape
    ///
    /// This function must be called by the derived class every time
    /// the shape's points change (i.e. the result of either
    /// getPointCount or getPoint is different).
    ///
    ////////////////////////////////////////////////////////////
    void update(const sf::Vector2f* points, std::size_t pointCount);

private:
    friend RenderTarget;

    ////////////////////////////////////////////////////////////
    /// \brief Draws the shape on `renderTarget` with the given `texture` and `states`
    ///
    ////////////////////////////////////////////////////////////
    void drawOnto(RenderTarget& renderTarget, const Texture* texture, const RenderStates& states) const;

    ////////////////////////////////////////////////////////////
    /// \brief Update the fill vertices' color
    ///
    ////////////////////////////////////////////////////////////
    void updateFillColors();

    ////////////////////////////////////////////////////////////
    /// \brief Update the fill vertices' texture coordinates
    ///
    ////////////////////////////////////////////////////////////
    void updateTexCoords();

    ////////////////////////////////////////////////////////////
    /// \brief Update the outline vertices' position
    ///
    ////////////////////////////////////////////////////////////
    void updateOutline();

    ////////////////////////////////////////////////////////////
    /// \brief Update the outline vertices' color
    ///
    ////////////////////////////////////////////////////////////
    void updateOutlineColors();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 128> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Shape
/// \ingroup graphics
///
/// sf::Shape is a non-polymorphic base class that allows to
/// define and display a custom convex shape on a render target.
///
/// In addition to the attributes provided by the specialized
/// shape classes, a shape always has the following attributes:
/// \li a texture
/// \li a texture rectangle
/// \li a fill color
/// \li an outline color
/// \li an outline thickness
///
/// Each feature is optional, and can be disabled easily:
/// \li the texture can be null
/// \li the fill/outline colors can be sf::Color::Transparent
/// \li the outline thickness can be zero
///
/// \see sf::RectangleShape, sf::CircleShape, sf::ConvexShape, sf::Transformable
///
////////////////////////////////////////////////////////////
