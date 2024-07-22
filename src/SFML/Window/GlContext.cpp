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
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/GlContext.hpp>
#include <SFML/Window/GlContextTypeImpl.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Err.hpp>

#include <glad/gl.h>

#include <ios>

#include <cctype>
#include <cstdlib>
#include <cstring>


namespace sf::priv
{
////////////////////////////////////////////////////////////
GlContext::~GlContext()
{
    m_graphicsContext.onGlContextDestroyed(*this);
}


////////////////////////////////////////////////////////////
const ContextSettings& GlContext::getSettings() const
{
    return m_settings;
}


////////////////////////////////////////////////////////////
[[nodiscard]] std::uint64_t GlContext::getId() const
{
    return m_id;
}


////////////////////////////////////////////////////////////
GlContext::GlContext(GraphicsContext& graphicsContext, std::uint64_t id, const ContextSettings& settings) :
m_settings(settings),
m_graphicsContext(graphicsContext),
m_id{id}
{
}


////////////////////////////////////////////////////////////
int GlContext::evaluateFormat(
    unsigned int           bitsPerPixel,
    const ContextSettings& settings,
    int                    colorBits,
    int                    depthBits,
    int                    stencilBits,
    int                    antialiasing,
    bool                   accelerated,
    bool                   sRgb)
{
    int colorDiff        = static_cast<int>(bitsPerPixel) - colorBits;
    int depthDiff        = static_cast<int>(settings.depthBits) - depthBits;
    int stencilDiff      = static_cast<int>(settings.stencilBits) - stencilBits;
    int antialiasingDiff = static_cast<int>(settings.antialiasingLevel) - antialiasing;

    // Weight sub-scores so that better settings don't score equally as bad as worse settings
    colorDiff *= ((colorDiff > 0) ? 100'000 : 1);
    depthDiff *= ((depthDiff > 0) ? 100'000 : 1);
    stencilDiff *= ((stencilDiff > 0) ? 100'000 : 1);
    antialiasingDiff *= ((antialiasingDiff > 0) ? 100'000 : 1);

    // Aggregate the scores
    int score = std::abs(colorDiff) + std::abs(depthDiff) + std::abs(stencilDiff) + std::abs(antialiasingDiff);

    // If the user wants an sRGB capable format, try really hard to get one
    if (settings.sRgbCapable && !sRgb)
        score += 10'000'000;

    // Make sure we prefer hardware acceleration over features
    if (!accelerated)
        score += 100'000'000;

    return score;
}


////////////////////////////////////////////////////////////
bool GlContext::initialize(const GlContext& sharedGlContext, const ContextSettings& requestedSettings)
{
    SFML_BASE_ASSERT(m_graphicsContext.getActiveThreadLocalGlContextPtr() == this);

    const auto& derivedSharedGlContext = static_cast<const DerivedGlContextType&>(sharedGlContext);

    // Retrieve the context version number
    int majorVersion = 0;
    int minorVersion = 0;

    // Try the new way first
    auto glGetIntegervFunc = reinterpret_cast<glGetIntegervFuncType>(
        derivedSharedGlContext.getFunction("glGetIntegerv"));
    auto glGetErrorFunc  = reinterpret_cast<glGetErrorFuncType>(derivedSharedGlContext.getFunction("glGetError"));
    auto glGetStringFunc = reinterpret_cast<glGetStringFuncType>(derivedSharedGlContext.getFunction("glGetString"));
    auto glEnableFunc    = reinterpret_cast<glEnableFuncType>(derivedSharedGlContext.getFunction("glEnable"));
    auto glIsEnabledFunc = reinterpret_cast<glIsEnabledFuncType>(derivedSharedGlContext.getFunction("glIsEnabled"));

    if (!glGetIntegervFunc || !glGetErrorFunc || !glGetStringFunc || !glEnableFunc || !glIsEnabledFunc)
    {
        err() << "Could not load necessary function to initialize OpenGL context";
        return false;
    }

// These functions are expected to fail, but we don't want to pollute the state of `glGetError` so we have to call it anyway
#define SFML_PRIV_GLCHECK_INNER(...) \
    do                               \
    {                                \
        __VA_ARGS__;                 \
        (void)glGetErrorFunc();      \
    } while (false)

    glGetIntegervFunc(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegervFunc(GL_MINOR_VERSION, &minorVersion);

    if (glGetErrorFunc() != GL_INVALID_ENUM)
    {
        m_settings.majorVersion = static_cast<unsigned int>(majorVersion);
        m_settings.minorVersion = static_cast<unsigned int>(minorVersion);
    }
    else
    {
        // Try the old way

        // If we can't get the version number, assume 1.1
        m_settings.majorVersion = 1;
        m_settings.minorVersion = 1;

        if (const char* version = reinterpret_cast<const char*>(glGetStringFunc(GL_VERSION)))
        {
            // OpenGL ES Common Lite profile: The beginning of the returned string is "OpenGL ES-CL major.minor"
            // OpenGL ES Common profile:      The beginning of the returned string is "OpenGL ES-CM major.minor"
            // OpenGL ES Full profile:        The beginning of the returned string is "OpenGL ES major.minor"
            // Desktop OpenGL:                The beginning of the returned string is "major.minor"

            // Helper to parse OpenGL version strings
            static const auto parseVersionString =
                [](const char* versionString, const char* prefix, unsigned int& major, unsigned int& minor)
            {
                const std::size_t prefixLength = std::strlen(prefix);

                if ((std::strlen(versionString) >= (prefixLength + 3)) &&
                    (std::strncmp(versionString, prefix, prefixLength) == 0) && std::isdigit(versionString[prefixLength]) &&
                    (versionString[prefixLength + 1] == '.') && std::isdigit(versionString[prefixLength + 2]))
                {
                    major = static_cast<unsigned int>(versionString[prefixLength] - '0');
                    minor = static_cast<unsigned int>(versionString[prefixLength + 2] - '0');

                    return true;
                }

                return false;
            };

            if (!parseVersionString(version, "OpenGL ES-CL ", m_settings.majorVersion, m_settings.minorVersion) &&
                !parseVersionString(version, "OpenGL ES-CM ", m_settings.majorVersion, m_settings.minorVersion) &&
                !parseVersionString(version, "OpenGL ES ", m_settings.majorVersion, m_settings.minorVersion) &&
                !parseVersionString(version, "", m_settings.majorVersion, m_settings.minorVersion))
            {
                err() << "Unable to parse OpenGL version string: \"" << version << '"' << ", defaulting to 1.1";
            }
        }
        else
        {
            err() << "Unable to retrieve OpenGL version string, defaulting to 1.1";
        }
    }

    // 3.0 contexts only deprecate features, but do not remove them yet
    // 3.1 contexts remove features if ARB_compatibility is not present
    // 3.2+ contexts remove features only if a core profile is requested

    // If the context was created with wglCreateContext, it is guaranteed to be compatibility.
    // If a 3.0 context was created with wglCreateContextAttribsARB, it is guaranteed to be compatibility.
    // If a 3.1 context was created with wglCreateContextAttribsARB, the compatibility flag
    // is set only if ARB_compatibility is present
    // If a 3.2+ context was created with wglCreateContextAttribsARB, the compatibility flag
    // would have been set correctly already depending on whether ARB_create_context_profile is supported.

    // If the user requests a 3.0 context, it will be a compatibility context regardless of the requested profile.
    // If the user requests a 3.1 context and its creation was successful, the specification
    // states that it will not be a compatibility profile context regardless of the requested
    // profile unless ARB_compatibility is present.

    m_settings.attributeFlags = ContextSettings::Attribute::Default;

    if (m_settings.majorVersion >= 3)
    {
        // Retrieve the context flags
        int flags = 0;
        SFML_PRIV_GLCHECK_INNER(glGetIntegervFunc(GL_CONTEXT_FLAGS, &flags));

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
            m_settings.attributeFlags |= ContextSettings::Attribute::Debug;

        if ((m_settings.majorVersion == 3) && (m_settings.minorVersion == 1))
        {
            m_settings.attributeFlags |= ContextSettings::Attribute::Core;

            auto glGetStringiFunc = reinterpret_cast<glGetStringiFuncType>(
                derivedSharedGlContext.getFunction("glGetStringi"));

            if (glGetStringiFunc)
            {
                int numExtensions = 0;
                SFML_PRIV_GLCHECK_INNER(glGetIntegervFunc(GL_NUM_EXTENSIONS, &numExtensions));

                for (unsigned int i = 0; i < static_cast<unsigned int>(numExtensions); ++i)
                {
                    const char* extensionString = reinterpret_cast<const char*>(glGetStringiFunc(GL_EXTENSIONS, i));

                    if (std::strstr(extensionString, "GL_ARB_compatibility"))
                    {
                        m_settings.attributeFlags &= ~ContextSettings::Attribute::Core;
                        break;
                    }
                }
            }
        }
        else if ((m_settings.majorVersion > 3) || (m_settings.minorVersion >= 2))
        {
            // Retrieve the context profile
            int profile = 0;
            SFML_PRIV_GLCHECK_INNER(glGetIntegervFunc(GL_CONTEXT_PROFILE_MASK, &profile));

            if (profile & GL_CONTEXT_CORE_PROFILE_BIT)
                m_settings.attributeFlags |= ContextSettings::Attribute::Core;
        }
    }

#undef SFML_PRIV_GLCHECK_INNER

    // Enable anti-aliasing if requested by the user and supported
    if ((requestedSettings.antialiasingLevel > 0) && (m_settings.antialiasingLevel > 0))
    {
        glEnableFunc(GL_MULTISAMPLE);
    }
    else
    {
        m_settings.antialiasingLevel = 0;
    }

    // Enable sRGB if requested by the user and supported
    if (requestedSettings.sRgbCapable && m_settings.sRgbCapable)
    {
        glEnableFunc(GL_FRAMEBUFFER_SRGB);

        // Check to see if the enable was successful
        if (glIsEnabledFunc(GL_FRAMEBUFFER_SRGB) == GL_FALSE)
        {
            err() << "Warning: Failed to enable GL_FRAMEBUFFER_SRGB";
            m_settings.sRgbCapable = false;
        }
    }
    else
    {
        m_settings.sRgbCapable = false;
    }

    return true;
}


////////////////////////////////////////////////////////////
void GlContext::checkSettings(const ContextSettings& requestedSettings) const
{
    // Perform checks to inform the user if they are getting a context they might not have expected
    const int version = static_cast<int>(m_settings.majorVersion * 10u + m_settings.minorVersion);
    const int requestedVersion = static_cast<int>(requestedSettings.majorVersion * 10u + requestedSettings.minorVersion);

    if ((m_settings.attributeFlags != requestedSettings.attributeFlags) || (version < requestedVersion) ||
        (m_settings.stencilBits < requestedSettings.stencilBits) ||
        (m_settings.antialiasingLevel < requestedSettings.antialiasingLevel) ||
        (m_settings.depthBits < requestedSettings.depthBits) || (!m_settings.sRgbCapable && requestedSettings.sRgbCapable))
    {
        err() << "Warning: The created OpenGL context does not fully meet the settings that were requested" << '\n'
              << "Requested: version = " << requestedSettings.majorVersion << "." << requestedSettings.minorVersion
              << " ; depth bits = " << requestedSettings.depthBits << " ; stencil bits = " << requestedSettings.stencilBits
              << " ; AA level = " << requestedSettings.antialiasingLevel << std::boolalpha << " ; core = "
              << ((requestedSettings.attributeFlags & ContextSettings::Attribute::Core) != ContextSettings::Attribute{0u})
              << " ; debug = "
              << ((requestedSettings.attributeFlags & ContextSettings::Attribute::Debug) != ContextSettings::Attribute{0u})
              << " ; sRGB = " << requestedSettings.sRgbCapable << std::noboolalpha << '\n'
              << "Created: version = " << m_settings.majorVersion << "." << m_settings.minorVersion
              << " ; depth bits = " << m_settings.depthBits << " ; stencil bits = " << m_settings.stencilBits
              << " ; AA level = " << m_settings.antialiasingLevel << std::boolalpha << " ; core = "
              << ((m_settings.attributeFlags & ContextSettings::Attribute::Core) != ContextSettings::Attribute{0u})
              << " ; debug = "
              << ((m_settings.attributeFlags & ContextSettings::Attribute::Debug) != ContextSettings::Attribute{0u})
              << " ; sRGB = " << m_settings.sRgbCapable << std::noboolalpha;
    }
}

} // namespace sf::priv
