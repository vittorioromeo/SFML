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
#include <SFML/Config.hpp>

#include <SFML/Window/ContextSettings.hpp>

#include <SFML/System/Vector2.hpp>

#include <SFML/Base/InPlacePImpl.hpp>
#include <SFML/Base/UniquePtr.hpp>


namespace sf
{
using GlFunctionPointer = void (*)();
class GraphicsContext;
} // namespace sf


namespace sf::priv
{
class WindowImpl;

////////////////////////////////////////////////////////////
/// \brief Abstract class representing an OpenGL context
///
////////////////////////////////////////////////////////////
class [[nodiscard]] GlContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~GlContext();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    GlContext(const GlContext&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    GlContext& operator=(const GlContext&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Get the settings of the context
    ///
    /// Note that these settings may be different than the ones
    /// passed to the constructor; they are indeed adjusted if the
    /// original settings are not directly supported by the system.
    ///
    /// \return Structure containing the settings
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const ContextSettings& getSettings() const;

    ////////////////////////////////////////////////////////////
    /// \brief Display what has been rendered to the context so far
    ///
    ////////////////////////////////////////////////////////////
    virtual void display() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable vertical synchronization
    ///
    /// Activating vertical synchronization will limit the number
    /// of frames displayed to the refresh rate of the monitor.
    /// This can avoid some visual artifacts, and limit the framerate
    /// to a good value (but not constant across different computers).
    ///
    /// \param enabled True to enable v-sync, false to deactivate
    ///
    ////////////////////////////////////////////////////////////
    virtual void setVerticalSyncEnabled(bool enabled) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Perform various initializations after the context construction
    /// \param requestedSettings Requested settings during context creation
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool initialize(const GlContext& sharedGlContext, const ContextSettings& requestedSettings);

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// This constructor is meant for derived classes only.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit GlContext(GraphicsContext& graphicsContext, std::uint64_t id, const ContextSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Activate the context as the current target
    ///        for rendering
    ///
    /// \param current Whether to make the context current or no longer current
    ///
    /// \return True on success, false if any error happened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool makeCurrent(bool current) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Evaluate a pixel format configuration
    ///
    /// This functions can be used by implementations that have
    /// several valid formats and want to get the best one.
    /// A score is returned for the given configuration: the
    /// lower the score is, the better the configuration is.
    ///
    /// \param bitsPerPixel Requested pixel depth (bits per pixel)
    /// \param settings     Requested additional settings
    /// \param colorBits    Color bits of the configuration to evaluate
    /// \param depthBits    Depth bits of the configuration to evaluate
    /// \param stencilBits  Stencil bits of the configuration to evaluate
    /// \param antialiasing Antialiasing level of the configuration to evaluate
    /// \param accelerated  Whether the pixel format is hardware accelerated
    /// \param sRgb         Whether the pixel format is sRGB capable
    ///
    /// \return Score of the configuration
    ///
    ////////////////////////////////////////////////////////////
    static int evaluateFormat(unsigned int           bitsPerPixel,
                              const ContextSettings& settings,
                              int                    colorBits,
                              int                    depthBits,
                              int                    stencilBits,
                              int                    antialiasing,
                              bool                   accelerated,
                              bool                   sRgb);

    ////////////////////////////////////////////////////////////
    /// \brief Get the OpenGL context ID for this context
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::uint64_t getId() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    ContextSettings  m_settings; //!< Creation settings of the context
    GraphicsContext& m_graphicsContext;

private:
    friend GraphicsContext;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the context is compatible with the requested settings
    /// \param requestedSettings Requested settings during context creation
    ///
    ////////////////////////////////////////////////////////////
    void checkSettings(const ContextSettings& requestedSettings) const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    const std::uint64_t m_id;
};

} // namespace sf::priv
