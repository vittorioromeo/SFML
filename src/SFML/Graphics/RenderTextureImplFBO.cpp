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
#include <SFML/Graphics/RenderTextureImplFBO.hpp>

#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Macros.hpp>
#include <SFML/Base/UniquePtr.hpp>

#include <unordered_map>

#include <cstdint>


namespace
{
////////////////////////////////////////////////////////////
void deleteFrameBuffer(unsigned int id)
{
    glCheck(GLEXT_glDeleteFramebuffers(1, &id));
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct RenderTextureImplFBO::Impl
{
    explicit Impl(GraphicsContext& theGraphicsContext) : graphicsContext(&theGraphicsContext)
    {
    }

    using FrameBufferIdMap = std::unordered_map<std::uint64_t, unsigned int>;

    GraphicsContext* graphicsContext; //!< The graphics context

    FrameBufferIdMap frameBuffers; //!< OpenGL frame buffer objects per context
    FrameBufferIdMap multisampleFrameBuffers; //!< base::Optional per-context OpenGL frame buffer objects with multisample attachments

    unsigned int depthStencilBuffer{}; //!< base::Optional depth/stencil buffer attached to the frame buffer
    unsigned int colorBuffer{};        //!< base::Optional multisample color buffer attached to the frame buffer
    Vector2u     size;                 //!< Width and height of the attachments
    unsigned int textureId{};          //!< The ID of the texture to attach to the FBO
    bool         multisample{};        //!< Whether we have to create a multisample frame buffer as well
    bool         depth{};              //!< Whether we have depth attachment
    bool         stencil{};            //!< Whether we have stencil attachment
    bool         sRgb{};               //!< Whether we need to encode drawn pixels into sRGB color space
};


////////////////////////////////////////////////////////////
RenderTextureImplFBO::RenderTextureImplFBO(GraphicsContext& graphicsContext) : m_impl(graphicsContext)
{
}


////////////////////////////////////////////////////////////
RenderTextureImplFBO::~RenderTextureImplFBO()
{
    SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    // Destroy the color buffer
    if (m_impl->colorBuffer)
    {
        const GLuint colorBuffer = m_impl->colorBuffer;
        glCheck(GLEXT_glDeleteRenderbuffers(1, &colorBuffer));
    }

    // Destroy the depth/stencil buffer
    if (m_impl->depthStencilBuffer)
    {
        const GLuint depthStencilBuffer = m_impl->depthStencilBuffer;
        glCheck(GLEXT_glDeleteRenderbuffers(1, &depthStencilBuffer));
    }

    // Unregister FBOs with the contexts if they haven't already been destroyed
    for (const auto& [glContextId, frameBufferId] : m_impl->frameBuffers)
        m_impl->graphicsContext->unregisterUnsharedFrameBuffer(glContextId, frameBufferId);

    for (const auto& [glContextId, multisampleFrameBufferId] : m_impl->multisampleFrameBuffers)
        m_impl->graphicsContext->unregisterUnsharedFrameBuffer(glContextId, multisampleFrameBufferId);
}


////////////////////////////////////////////////////////////
bool RenderTextureImplFBO::isAvailable(GraphicsContext& graphicsContext)
{
    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());
    return GLEXT_framebuffer_object != 0;
}


////////////////////////////////////////////////////////////
unsigned int RenderTextureImplFBO::getMaximumAntialiasingLevel([[maybe_unused]] GraphicsContext& graphicsContext)
{
#ifdef SFML_OPENGL_ES

    return 0;

#else

    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());
    return static_cast<unsigned int>(getGLInteger(GLEXT_GL_MAX_SAMPLES));

#endif
}


////////////////////////////////////////////////////////////
void RenderTextureImplFBO::unbind()
{
    glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, 0));
}


////////////////////////////////////////////////////////////
bool RenderTextureImplFBO::create(Vector2u size, unsigned int textureId, const ContextSettings& settings)
{
    // Store the dimensions
    m_impl->size = size;

    {
        SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());

        if (settings.antialiasingLevel && !(GLEXT_framebuffer_multisample && GLEXT_framebuffer_blit))
            return false;

        m_impl->sRgb = settings.sRgbCapable && GL_EXT_texture_sRGB;

        // Check if the requested anti-aliasing level is supported
        if (settings.antialiasingLevel)
        {
            const auto samples = getGLInteger(GLEXT_GL_MAX_SAMPLES);

            if (settings.antialiasingLevel > static_cast<unsigned int>(samples))
            {
                err() << "Impossible to create render texture (unsupported anti-aliasing level)"
                      << " Requested: " << settings.antialiasingLevel << " Maximum supported: " << samples;

                return false;
            }
        }

        if (!settings.antialiasingLevel)
        {
            // Create the depth/stencil buffer if requested
            if (settings.stencilBits && settings.depthBits)
            {
                if (!GLEXT_packed_depth_stencil)
                {
                    err() << "Impossible to create render texture (combined depth/stencil buffer not supported)";
                    return false;
                }

                GLuint depthStencil = 0;
                glCheck(GLEXT_glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached depth/stencil "
                             "buffer)";
                    return false;
                }
                glCheck(GLEXT_glBindRenderbuffer(GLEXT_GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(GLEXT_glRenderbufferStorage(GLEXT_GL_RENDERBUFFER,
                                                    GL_DEPTH24_STENCIL8,
                                                    static_cast<GLsizei>(size.x),
                                                    static_cast<GLsizei>(size.y)));

                m_impl->depth   = true;
                m_impl->stencil = true;
            }
            else if (settings.depthBits)
            {
                GLuint depthStencil = 0;
                glCheck(GLEXT_glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached depth buffer)";
                    return false;
                }
                glCheck(GLEXT_glBindRenderbuffer(GLEXT_GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(GLEXT_glRenderbufferStorage(GLEXT_GL_RENDERBUFFER,
                                                    GLEXT_GL_DEPTH_COMPONENT,
                                                    static_cast<GLsizei>(size.x),
                                                    static_cast<GLsizei>(size.y)));

                m_impl->depth   = true;
                m_impl->stencil = false;
            }
            else if (settings.stencilBits)
            {
                GLuint depthStencil = 0;
                glCheck(GLEXT_glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached stencil buffer)";
                    return false;
                }
                glCheck(GLEXT_glBindRenderbuffer(GLEXT_GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(GLEXT_glRenderbufferStorage(GLEXT_GL_RENDERBUFFER,
                                                    GLEXT_GL_STENCIL_INDEX8,
                                                    static_cast<GLsizei>(size.x),
                                                    static_cast<GLsizei>(size.y)));

                m_impl->depth   = false;
                m_impl->stencil = true;
            }
        }
        else
        {
            // Create the multisample color buffer
            GLuint color = 0;
            glCheck(GLEXT_glGenRenderbuffers(1, &color));
            m_impl->colorBuffer = color;
            if (!m_impl->colorBuffer)
            {
                err() << "Impossible to create render texture (failed to create the attached multisample color "
                         "buffer)";
                return false;
            }
            glCheck(GLEXT_glBindRenderbuffer(GLEXT_GL_RENDERBUFFER, m_impl->colorBuffer));
            glCheck(GLEXT_glRenderbufferStorageMultisample(GLEXT_GL_RENDERBUFFER,
                                                           static_cast<GLsizei>(settings.antialiasingLevel),
                                                           m_impl->sRgb ? GL_SRGB8_ALPHA8_EXT : GL_RGBA,
                                                           static_cast<GLsizei>(size.x),
                                                           static_cast<GLsizei>(size.y)));

            // Create the multisample depth/stencil buffer if requested
            if (settings.stencilBits && settings.depthBits)
            {
                GLuint depthStencil = 0;
                glCheck(GLEXT_glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached multisample "
                             "depth/stencil buffer)";
                    return false;
                }
                glCheck(GLEXT_glBindRenderbuffer(GLEXT_GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(GLEXT_glRenderbufferStorageMultisample(GLEXT_GL_RENDERBUFFER,
                                                               static_cast<GLsizei>(settings.antialiasingLevel),
                                                               GL_DEPTH24_STENCIL8,
                                                               static_cast<GLsizei>(size.x),
                                                               static_cast<GLsizei>(size.y)));

                m_impl->depth   = true;
                m_impl->stencil = true;
            }
            else if (settings.depthBits)
            {
                GLuint depthStencil = 0;
                glCheck(GLEXT_glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached multisample "
                             "depth "
                             "buffer)";
                    return false;
                }
                glCheck(GLEXT_glBindRenderbuffer(GLEXT_GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(GLEXT_glRenderbufferStorageMultisample(GLEXT_GL_RENDERBUFFER,
                                                               static_cast<GLsizei>(settings.antialiasingLevel),
                                                               GLEXT_GL_DEPTH_COMPONENT,
                                                               static_cast<GLsizei>(size.x),
                                                               static_cast<GLsizei>(size.y)));

                m_impl->depth   = true;
                m_impl->stencil = false;
            }
            else if (settings.stencilBits)
            {
                GLuint depthStencil = 0;
                glCheck(GLEXT_glGenRenderbuffers(1, &depthStencil));
                m_impl->depthStencilBuffer = depthStencil;
                if (!m_impl->depthStencilBuffer)
                {
                    err() << "Impossible to create render texture (failed to create the attached multisample "
                             "stencil buffer)";
                    return false;
                }
                glCheck(GLEXT_glBindRenderbuffer(GLEXT_GL_RENDERBUFFER, m_impl->depthStencilBuffer));
                glCheck(GLEXT_glRenderbufferStorageMultisample(GLEXT_GL_RENDERBUFFER,
                                                               static_cast<GLsizei>(settings.antialiasingLevel),
                                                               GLEXT_GL_STENCIL_INDEX8,
                                                               static_cast<GLsizei>(size.x),
                                                               static_cast<GLsizei>(size.y)));

                m_impl->depth   = false;
                m_impl->stencil = true;
            }

            m_impl->multisample = true;
        }
    }

    // Save our texture ID in order to be able to attach it to an FBO at any time
    m_impl->textureId = textureId;

    // We can't create an FBO now if there is no active context
    if (!m_impl->graphicsContext->hasActiveThreadLocalGlContext())
        return true;

#ifndef SFML_OPENGL_ES

    // Save the current bindings so we can restore them after we are done
    const auto readFramebuffer = getGLInteger(GLEXT_GL_READ_FRAMEBUFFER_BINDING);
    const auto drawFramebuffer = getGLInteger(GLEXT_GL_DRAW_FRAMEBUFFER_BINDING);

    if (createFrameBuffer())
    {
        // Restore previously bound framebuffers
        glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_READ_FRAMEBUFFER, static_cast<GLuint>(readFramebuffer)));
        glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFramebuffer)));

        return true;
    }

#else

    // Save the current binding so we can restore them after we are done
    const auto frameBuffer = getGLInteger(GLEXT_GL_DRAW_FRAMEBUFFER_BINDING);

    if (createFrameBuffer())
    {
        // Restore previously bound framebuffer
        glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, static_cast<GLuint>(frameBuffer)));

        return true;
    }

#endif

    return false;
}


////////////////////////////////////////////////////////////
bool RenderTextureImplFBO::createFrameBuffer()
{
    // Create the framebuffer object
    GLuint frameBufferId{};
    glCheck(GLEXT_glGenFramebuffers(1, &frameBufferId));

    if (!frameBufferId)
    {
        err() << "Impossible to create render texture (failed to create the frame buffer object)";
        return false;
    }

    glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, frameBufferId));

    // Link the depth/stencil renderbuffer to the frame buffer
    if (!m_impl->multisample && m_impl->depthStencilBuffer)
    {
        if (m_impl->depth)
        {
            glCheck(GLEXT_glFramebufferRenderbuffer(GLEXT_GL_FRAMEBUFFER,
                                                    GLEXT_GL_DEPTH_ATTACHMENT,
                                                    GLEXT_GL_RENDERBUFFER,
                                                    m_impl->depthStencilBuffer));
        }

        if (m_impl->stencil)
        {
            glCheck(GLEXT_glFramebufferRenderbuffer(GLEXT_GL_FRAMEBUFFER,
                                                    GLEXT_GL_STENCIL_ATTACHMENT,
                                                    GLEXT_GL_RENDERBUFFER,
                                                    m_impl->depthStencilBuffer));
        }
    }

    // Link the texture to the frame buffer
    glCheck(
        GLEXT_glFramebufferTexture2D(GLEXT_GL_FRAMEBUFFER, GLEXT_GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_impl->textureId, 0));

    // A final check, just to be sure...
    GLenum status = 0;
    glCheck(status = GLEXT_glCheckFramebufferStatus(GLEXT_GL_FRAMEBUFFER));
    if (status != GLEXT_GL_FRAMEBUFFER_COMPLETE)
    {
        glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, 0));
        err() << "Impossible to create render texture (failed to link the target texture to the frame buffer)";
        return false;
    }

    // Get current GL context id
    const std::uint64_t glContextId = m_impl->graphicsContext->getActiveThreadLocalGlContextId();

    // Insert the FBO into our map
    m_impl->frameBuffers.emplace(glContextId, frameBufferId);

    // Register the object with the current context so it is automatically destroyed
    m_impl->graphicsContext->registerUnsharedFrameBuffer(glContextId, frameBufferId, &deleteFrameBuffer);

#ifndef SFML_OPENGL_ES

    if (m_impl->multisample)
    {
        // Create the multisample framebuffer object
        GLuint multisampleFrameBufferId{};
        glCheck(GLEXT_glGenFramebuffers(1, &multisampleFrameBufferId));

        if (!multisampleFrameBufferId)
        {
            err() << "Impossible to create render texture (failed to create the multisample frame buffer object)";
            return false;
        }
        glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, multisampleFrameBufferId));

        // Link the multisample color buffer to the frame buffer
        glCheck(GLEXT_glBindRenderbuffer(GLEXT_GL_RENDERBUFFER, m_impl->colorBuffer));
        glCheck(GLEXT_glFramebufferRenderbuffer(GLEXT_GL_FRAMEBUFFER,
                                                GLEXT_GL_COLOR_ATTACHMENT0,
                                                GLEXT_GL_RENDERBUFFER,
                                                m_impl->colorBuffer));

        // Link the depth/stencil renderbuffer to the frame buffer
        if (m_impl->depthStencilBuffer)
        {
            if (m_impl->depth)
            {
                glCheck(GLEXT_glFramebufferRenderbuffer(GLEXT_GL_FRAMEBUFFER,
                                                        GLEXT_GL_DEPTH_ATTACHMENT,
                                                        GLEXT_GL_RENDERBUFFER,
                                                        m_impl->depthStencilBuffer));
            }

            if (m_impl->stencil)
            {
                glCheck(GLEXT_glFramebufferRenderbuffer(GLEXT_GL_FRAMEBUFFER,
                                                        GLEXT_GL_STENCIL_ATTACHMENT,
                                                        GLEXT_GL_RENDERBUFFER,
                                                        m_impl->depthStencilBuffer));
            }
        }

        // A final check, just to be sure...
        glCheck(status = GLEXT_glCheckFramebufferStatus(GLEXT_GL_FRAMEBUFFER));
        if (status != GLEXT_GL_FRAMEBUFFER_COMPLETE)
        {
            glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, 0));
            err() << "Impossible to create render texture (failed to link the render buffers to the multisample "
                     "frame "
                     "buffer)";
            return false;
        }

        // Insert the FBO into our map
        m_impl->multisampleFrameBuffers.emplace(glContextId, multisampleFrameBufferId);

        // Register the object with the current context so it is automatically destroyed
        m_impl->graphicsContext->registerUnsharedFrameBuffer(glContextId, multisampleFrameBufferId, &deleteFrameBuffer);
    }

#endif

    return true;
}


////////////////////////////////////////////////////////////
bool RenderTextureImplFBO::activate(bool active)
{
    // Unbind the FBO if requested
    if (!active)
    {
        glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, 0));
        return true;
    }

    SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());
    const std::uint64_t glContextId = m_impl->graphicsContext->getActiveThreadLocalGlContextId();

    // Lookup the FBO corresponding to the currently active context
    // If none is found, there is no FBO corresponding to the
    // currently active context so we will have to create a new FBO
    if (m_impl->multisample)
    {
        const auto it = m_impl->multisampleFrameBuffers.find(glContextId);

        if (it != m_impl->multisampleFrameBuffers.end())
        {
            glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, it->second));
            return true;
        }
    }
    else
    {
        const auto it = m_impl->frameBuffers.find(glContextId);

        if (it != m_impl->frameBuffers.end())
        {
            glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_FRAMEBUFFER, it->second));
            return true;
        }
    }

    return createFrameBuffer();
}


////////////////////////////////////////////////////////////
bool RenderTextureImplFBO::isSrgb() const
{
    return m_impl->sRgb;
}


////////////////////////////////////////////////////////////
void RenderTextureImplFBO::updateTexture(unsigned int)
{
    // If multisampling is enabled, we need to resolve by blitting
    // from our FBO with multisample renderbuffer attachments
    // to our FBO to which our target texture is attached

#ifndef SFML_OPENGL_ES

    // In case of multisampling, make sure both FBOs
    // are already available within the current context
    if (m_impl->multisample && m_impl->size.x && m_impl->size.y && activate(true))
    {
        const std::uint64_t glContextId = m_impl->graphicsContext->getActiveThreadLocalGlContextId();

        const auto frameBufferIt = m_impl->frameBuffers.find(glContextId);
        const auto multisampleIt = m_impl->multisampleFrameBuffers.find(glContextId);

        if ((frameBufferIt != m_impl->frameBuffers.end()) && (multisampleIt != m_impl->multisampleFrameBuffers.end()))
        {
            // Scissor testing affects framebuffer blits as well
            // Since we don't want scissor testing to interfere with our copying, we temporarily disable it for the blit if it is enabled
            GLboolean scissorEnabled = GL_FALSE;
            glCheck(glGetBooleanv(GL_SCISSOR_TEST, &scissorEnabled));

            if (scissorEnabled == GL_TRUE)
                glCheck(glDisable(GL_SCISSOR_TEST));

            // Set up the blit target (draw framebuffer) and blit (from the read framebuffer, our multisample FBO)
            glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_DRAW_FRAMEBUFFER, frameBufferIt->second));
            glCheck(GLEXT_glBlitFramebuffer(0,
                                            0,
                                            static_cast<GLint>(m_impl->size.x),
                                            static_cast<GLint>(m_impl->size.y),
                                            0,
                                            0,
                                            static_cast<GLint>(m_impl->size.x),
                                            static_cast<GLint>(m_impl->size.y),
                                            GL_COLOR_BUFFER_BIT,
                                            GL_NEAREST));
            glCheck(GLEXT_glBindFramebuffer(GLEXT_GL_DRAW_FRAMEBUFFER, multisampleIt->second));

            // Re-enable scissor testing if it was previously enabled
            if (scissorEnabled == GL_TRUE)
                glCheck(glEnable(GL_SCISSOR_TEST));
        }
    }

#endif // SFML_OPENGL_ES
}

} // namespace sf::priv
