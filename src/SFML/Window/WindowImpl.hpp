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

#include <SFML/Window/Event.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <SFML/Window/WindowHandle.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <SFML/Base/InPlacePImpl.hpp>
#include <SFML/Base/Optional.hpp>
#include <SFML/Base/UniquePtr.hpp>

#include <cstdint>


namespace sf
{
struct ContextSettings;
class String;
class Time;
class VideoMode;

namespace Vulkan
{
struct VulkanSurfaceData;
} // namespace Vulkan

namespace priv
{
class CursorImpl;

////////////////////////////////////////////////////////////
/// \brief Abstract base class for OS-specific window implementation
///
////////////////////////////////////////////////////////////
class WindowImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create a new window depending on the current OS
    ///
    /// \param mode  Video mode to use
    /// \param title Title of the window
    /// \param style Window style
    /// \param state Window state
    /// \param settings Additional settings for the underlying OpenGL context
    ///
    /// \return Pointer to the created window
    ///
    ////////////////////////////////////////////////////////////
    static base::UniquePtr<WindowImpl> create(VideoMode              mode,
                                              const String&          title,
                                              Style                  style,
                                              State                  state,
                                              const ContextSettings& settings);

    ////////////////////////////////////////////////////////////
    /// \brief Create a new window depending on to the current OS
    ///
    /// \param handle Platform-specific handle of the control
    ///
    /// \return Pointer to the created window
    ///
    ////////////////////////////////////////////////////////////
    static base::UniquePtr<WindowImpl> create(WindowHandle handle);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~WindowImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    WindowImpl(const WindowImpl&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    WindowImpl& operator=(const WindowImpl&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Change the joystick threshold, i.e. the value below which
    ///        no move event will be generated
    ///
    /// \param threshold New threshold, in range [0, 100]
    ///
    ////////////////////////////////////////////////////////////
    void setJoystickThreshold(float threshold);

    ////////////////////////////////////////////////////////////
    /// \brief Wait for and return the next available window event
    ///
    /// If there's no event available, this function calls the
    /// window's internal event processing function.
    ///
    /// \param timeout Maximum time to wait (`Time::Zero` for infinite)
    ///
    /// \return The event on success, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> waitEvent(Time timeout);

    ////////////////////////////////////////////////////////////
    /// \brief Return the next window event, if available
    ///
    /// If there's no event available, this function calls the
    /// window's internal event processing function.
    ///
    /// \return The event if available, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> pollEvent();

    ////////////////////////////////////////////////////////////
    /// \brief Get the OS-specific handle of the window
    ///
    /// \return Handle of the window
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual WindowHandle getNativeHandle() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the position of the window
    ///
    /// \return Position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual Vector2i getPosition() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the minimum window rendering region size
    ///
    /// \return Minimum size
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Vector2u> getMinimumSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the maximum window rendering region size
    ///
    /// \return Maximum size
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Vector2u> getMaximumSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Change the position of the window on screen
    ///
    /// \param position New position of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setPosition(Vector2i position) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Get the client size of the window
    ///
    /// \return Size of the window, in pixels
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual Vector2u getSize() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the size of the rendering region of the window
    ///
    /// \param size New size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setSize(Vector2u size) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Set the minimum window rendering region size
    ///
    /// Pass base::nullOpt to unset the minimum size
    ///
    /// \param minimumSize New minimum size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMinimumSize(const base::Optional<Vector2u>& minimumSize);

    ////////////////////////////////////////////////////////////
    /// \brief Set the maximum window rendering region size
    ///
    /// Pass base::nullOpt to unset the maximum size
    ///
    /// \param maximumSize New maximum size, in pixels
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMaximumSize(const base::Optional<Vector2u>& maximumSize);

    ////////////////////////////////////////////////////////////
    /// \brief Change the title of the window
    ///
    /// \param title New title
    ///
    ////////////////////////////////////////////////////////////
    virtual void setTitle(const String& title) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Change the window's icon
    ///
    /// \param size   Icon's width and height, in pixels
    /// \param pixels Pointer to the pixels in memory, format must be RGBA 32 bits
    ///
    ////////////////////////////////////////////////////////////
    virtual void setIcon(Vector2u size, const std::uint8_t* pixels) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the window
    ///
    /// \param visible True to show, false to hide
    ///
    ////////////////////////////////////////////////////////////
    virtual void setVisible(bool visible) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Show or hide the mouse cursor
    ///
    /// \param visible True to show, false to hide
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMouseCursorVisible(bool visible) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Grab or release the mouse cursor and keeps it from leaving
    ///
    /// \param grabbed True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMouseCursorGrabbed(bool grabbed) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Set the displayed cursor to a native system cursor
    ///
    /// \param cursor Native system cursor type to display
    ///
    ////////////////////////////////////////////////////////////
    virtual void setMouseCursor(const CursorImpl& cursor) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable automatic key-repeat
    ///
    /// \param enabled True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    virtual void setKeyRepeatEnabled(bool enabled) = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Request the current window to be made the active
    ///        foreground window
    ///
    ////////////////////////////////////////////////////////////
    virtual void requestFocus() = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Check whether the window has the input focus
    ///
    /// \return True if window has focus, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual bool hasFocus() const = 0;

    ////////////////////////////////////////////////////////////
    /// \brief Create a Vulkan rendering surface
    ///
    /// \param instance  Vulkan instance
    /// \param surface   Created surface
    /// \param allocator Allocator to use
    ///
    /// \return True if surface creation was successful, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool createVulkanSurface(const Vulkan::VulkanSurfaceData& vulkanSurfaceData) const;

protected:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    WindowImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Push a new event into the event queue
    ///
    /// This function is to be used by derived classes, to
    /// notify the SFML window that a new event was triggered
    /// by the system.
    ///
    /// \param event Event to push
    ///
    ////////////////////////////////////////////////////////////
    void pushEvent(const Event& event);

    ////////////////////////////////////////////////////////////
    /// \brief Process incoming events from the operating system
    ///
    ////////////////////////////////////////////////////////////
    virtual void processEvents() = 0;

private:
    struct JoystickStatesImpl;

    ////////////////////////////////////////////////////////////
    /// \return First event of the queue if available, `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Event> popEvent();

    ////////////////////////////////////////////////////////////
    /// \brief Read the joysticks state and generate the appropriate events
    ///
    ////////////////////////////////////////////////////////////
    void processJoystickEvents();

    ////////////////////////////////////////////////////////////
    /// \brief Read the sensors state and generate the appropriate events
    ///
    ////////////////////////////////////////////////////////////
    void processSensorEvents();

    ////////////////////////////////////////////////////////////
    /// \brief Read joystick, sensors, and OS state and populate event queue
    ///
    ////////////////////////////////////////////////////////////
    void populateEventQueue();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 512> m_impl; //!< Implementation details
};

} // namespace priv

} // namespace sf
