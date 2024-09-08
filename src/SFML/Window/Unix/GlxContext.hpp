#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/GlContext.hpp"
#include "SFML/Window/WindowEnums.hpp" // Prevent conflict with macro None from Xlib

#include <X11/Xlib.h>
#include <glad/glx.h>

#include <memory>


namespace sf
{
class WindowContext;
}


namespace sf::priv
{
class WindowImpl;

////////////////////////////////////////////////////////////
/// \brief Linux (GLX) implementation of OpenGL contexts
///
////////////////////////////////////////////////////////////
class GlxContext : public GlContext
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a new default context
    ///
    /// \param shared Context to share the new one with (can be a null pointer)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit GlxContext(WindowContext& windowContext, std::uint64_t id, GlxContext* shared);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context attached to a window
    ///
    /// \param shared       Context to share the new one with
    /// \param settings     Creation parameters
    /// \param owner        Pointer to the owner window
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit GlxContext(WindowContext&         windowContext,
                                      std::uint64_t          id,
                                      GlxContext*            shared,
                                      const ContextSettings& contextSettings,
                                      const WindowImpl&      owner,
                                      unsigned int           bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new context that embeds its own rendering target
    ///
    /// \param shared   Context to share the new one with
    /// \param settings Creation parameters
    /// \param size     Back buffer width and height, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit GlxContext(WindowContext&         windowContext,
                                      std::uint64_t          id,
                                      GlxContext*            shared,
                                      const ContextSettings& contextSettings,
                                      Vector2u               size);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~GlxContext() override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the address of an OpenGL function
    ///
    /// \param name Name of the function to get the address of
    ///
    /// \return Address of the OpenGL function, 0 on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] GlFunctionPointer getFunction(const char* name) const;

    ////////////////////////////////////////////////////////////
    /// \brief Activate the context as the current target for rendering
    ///
    /// \param current Whether to make the context current or no longer current
    ///
    /// \return True on success, false if any error happened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool makeCurrent(bool current) override;

    ////////////////////////////////////////////////////////////
    /// \brief Display what has been rendered to the context so far
    ///
    ////////////////////////////////////////////////////////////
    void display() override;

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
    void setVerticalSyncEnabled(bool enabled) override;

    ////////////////////////////////////////////////////////////
    /// \brief Select the best GLX visual for a given set of settings
    ///
    /// \param display      X display
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    /// \param settings     Requested context settings
    ///
    /// \return The best visual
    ///
    ////////////////////////////////////////////////////////////
    static XVisualInfo selectBestVisual(::Display* display, unsigned int bitsPerPixel, const ContextSettings& contextSettings);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Update the context visual settings from XVisualInfo
    ///
    /// \param visualInfo XVisualInfo to update settings from
    ///
    ////////////////////////////////////////////////////////////
    void updateSettingsFromVisualInfo(XVisualInfo* visualInfo);

    ////////////////////////////////////////////////////////////
    /// \brief Update the context visual settings from the window
    ///
    ////////////////////////////////////////////////////////////
    void updateSettingsFromWindow();

    ////////////////////////////////////////////////////////////
    /// \brief Create the context's drawing surface
    ///
    /// \param shared       Context to share the new one with (can be a null pointer)
    /// \param size         Back buffer width and height, in pixels
    /// \param bitsPerPixel Pixel depth, in bits per pixel
    ///
    ////////////////////////////////////////////////////////////
    void createSurface(GlxContext* shared, Vector2u size, unsigned int bitsPerPixel);

    ////////////////////////////////////////////////////////////
    /// \brief Create the context's drawing surface from an existing window
    ///
    /// \param window Window ID of the owning window
    ///
    ////////////////////////////////////////////////////////////
    void createSurface(::Window window);

    ////////////////////////////////////////////////////////////
    /// \brief Create the context
    ///
    /// \param shared Context to share the new one with (can be a null pointer)
    ///
    ////////////////////////////////////////////////////////////
    void createContext(GlxContext* shared);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    std::shared_ptr<Display> m_display;      ///< Connection to the X server
    ::Window                 m_window{};     ///< Window to which the context is attached
    GLXContext               m_context{};    ///< OpenGL context
    GLXPbuffer               m_pbuffer{};    ///< GLX pbuffer ID if one was created
    bool                     m_ownsWindow{}; ///< Do we own the window associated to the context?
};

} // namespace sf::priv
