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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderTextureImplDefault.hpp>
#include <SFML/Graphics/RenderTextureImplFBO.hpp>

#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Macros.hpp>
#include <SFML/Base/UniquePtr.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
RenderTexture::~RenderTexture() = default;


////////////////////////////////////////////////////////////
RenderTexture::RenderTexture(RenderTexture&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderTexture& RenderTexture::operator=(RenderTexture&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<RenderTexture> RenderTexture::create(GraphicsContext& graphicsContext, Vector2u size, const ContextSettings& settings)
{
    base::Optional<RenderTexture> result; // Use a single local variable for NRVO

    // Create the texture
    auto texture = sf::Texture::create(graphicsContext, size, settings.sRgbCapable);
    if (!texture.hasValue())
    {
        priv::err() << "Impossible to create render texture (failed to create the target texture)";
        return result; // Empty optional
    }

    auto& renderTexture = result.emplace(base::PassKey<RenderTexture>{}, graphicsContext, SFML_BASE_MOVE(*texture));

    // We disable smoothing by default for render textures
    renderTexture.setSmooth(false);

    // Create the implementation
    if (priv::RenderTextureImplFBO::isAvailable(graphicsContext))
    {
        // Use frame-buffer object (FBO)
        renderTexture.m_impl = base::makeUnique<priv::RenderTextureImplFBO>(graphicsContext);

        // Mark the texture as being a framebuffer object attachment
        renderTexture.m_texture.m_fboAttachment = true;
    }
    else
    {
        // Use default implementation
        renderTexture.m_impl = base::makeUnique<priv::RenderTextureImplDefault>(graphicsContext);
    }

    // Initialize the render texture
    // We pass the actual size of our texture since OpenGL ES requires that all attachments have identical sizes
    if (!renderTexture.m_impl->create(renderTexture.m_texture.m_actualSize, renderTexture.m_texture.m_texture, settings))
    {
        result.reset();
        return result; // Empty optional
    }

    // We can now initialize the render target part
    renderTexture.initialize();

    return result;
}


////////////////////////////////////////////////////////////
unsigned int RenderTexture::getMaximumAntialiasingLevel(GraphicsContext& graphicsContext)
{
    if (priv::RenderTextureImplFBO::isAvailable(graphicsContext))
    {
        return priv::RenderTextureImplFBO::getMaximumAntialiasingLevel(graphicsContext);
    }

    return priv::RenderTextureImplDefault::getMaximumAntialiasingLevel();
}


////////////////////////////////////////////////////////////
void RenderTexture::setSmooth(bool smooth)
{
    m_texture.setSmooth(smooth);
}


////////////////////////////////////////////////////////////
bool RenderTexture::isSmooth() const
{
    return m_texture.isSmooth();
}


////////////////////////////////////////////////////////////
void RenderTexture::setRepeated(bool repeated)
{
    m_texture.setRepeated(repeated);
}


////////////////////////////////////////////////////////////
bool RenderTexture::isRepeated() const
{
    return m_texture.isRepeated();
}


////////////////////////////////////////////////////////////
bool RenderTexture::generateMipmap()
{
    return m_texture.generateMipmap();
}


////////////////////////////////////////////////////////////
bool RenderTexture::setActive(bool active)
{
    // Update RenderTarget tracking
    if (m_impl->activate(active))
        return RenderTarget::setActive(active);

    return false;
}


////////////////////////////////////////////////////////////
void RenderTexture::display()
{
    if (priv::RenderTextureImplFBO::isAvailable(getGraphicsContext()))
    {
        // Perform a RenderTarget-only activation if we are using FBOs
        if (!RenderTarget::setActive())
            return;
    }
    else
    {
        // Perform a full activation if we are not using FBOs
        if (!setActive())
            return;
    }

    // Update the target texture
    m_impl->updateTexture(m_texture.m_texture);
    m_texture.m_pixelsFlipped = true;
    m_texture.invalidateMipmap();
}


////////////////////////////////////////////////////////////
Vector2u RenderTexture::getSize() const
{
    return m_texture.getSize();
}


////////////////////////////////////////////////////////////
bool RenderTexture::isSrgb() const
{
    return m_impl->isSrgb();
}


////////////////////////////////////////////////////////////
const Texture& RenderTexture::getTexture() const
{
    return m_texture;
}


////////////////////////////////////////////////////////////
RenderTexture::RenderTexture(base::PassKey<RenderTexture>&&, GraphicsContext& graphicsContext, Texture&& texture) :
RenderTarget(graphicsContext),
m_texture(SFML_BASE_MOVE(texture))
{
}


} // namespace sf
