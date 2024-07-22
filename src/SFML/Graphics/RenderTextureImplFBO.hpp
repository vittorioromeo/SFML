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
#include <SFML/Graphics/RenderTextureImpl.hpp>

#include <SFML/System/Vector2.hpp>

#include <SFML/Base/InPlacePImpl.hpp>


namespace sf
{
class GraphicsContext;
struct ContextSettings;

namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Specialization of RenderTextureImpl using the
///        FrameBuffer Object OpenGL extension
///
////////////////////////////////////////////////////////////
class RenderTextureImplFBO : public RenderTextureImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] RenderTextureImplFBO(GraphicsContext& graphicsContext);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~RenderTextureImplFBO() override;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the system supports FBOs or not
    ///
    /// \return True if FBO render textures are supported
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool isAvailable(GraphicsContext& graphicsContext);

    ////////////////////////////////////////////////////////////
    /// \brief Get the maximum anti-aliasing level supported by the system
    ///
    /// \return The maximum anti-aliasing level supported by the system
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static unsigned int getMaximumAntialiasingLevel(GraphicsContext& graphicsContext);

    ////////////////////////////////////////////////////////////
    /// \brief Unbind the currently bound FBO
    ///
    ////////////////////////////////////////////////////////////
    static void unbind();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Create the render texture implementation
    ///
    /// \param size       Width and height of the texture to render to
    /// \param textureId  OpenGL identifier of the target texture
    /// \param settings   Context settings to create render-texture with
    ///
    /// \return True if creation has been successful
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool create(Vector2u size, unsigned int textureId, const ContextSettings& settings) override;

    ////////////////////////////////////////////////////////////
    /// \brief Create an FBO in the current context
    ///
    /// \return True if creation has been successful
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool createFrameBuffer();

    ////////////////////////////////////////////////////////////
    /// \brief Activate or deactivate the render texture for rendering
    ///
    /// \param active True to activate, false to deactivate
    ///
    /// \return True on success, false on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool activate(bool active) override;

    ////////////////////////////////////////////////////////////
    /// \brief Tell if the render-texture will use sRGB encoding when drawing on it
    ///
    /// You can request sRGB encoding for a render-texture
    /// by having the sRgbCapable flag set for the context parameter of create() method
    ///
    /// \return True if the render-texture use sRGB encoding, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isSrgb() const override;

    ////////////////////////////////////////////////////////////
    /// \brief Update the pixels of the target texture
    ///
    /// \param textureId OpenGL identifier of the target texture
    ///
    ////////////////////////////////////////////////////////////
    void updateTexture(unsigned textureId) override;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 192> m_impl; //!< Implementation details
};

} // namespace priv
} // namespace sf
