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
#include <SFML/Window/Cursor.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Vulkan.hpp>
#include <SFML/Window/WindowBase.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <SFML/Window/WindowHandle.hpp>
#include <SFML/Window/WindowImpl.hpp>

#include <SFML/System/String.hpp>

#include <SFML/Base/Algorithm.hpp>
#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Optional.hpp>

#include <climits>
#include <cstdlib>


namespace sf
{
////////////////////////////////////////////////////////////
WindowBase::WindowBase(base::UniquePtr<priv::WindowImpl>&& impl) : m_impl(SFML_BASE_MOVE(impl))
{
    // Setup default behaviors (to get a consistent behavior across different implementations)
    setVisible(true);
    setMouseCursorVisible(true);
    setKeyRepeatEnabled(true);

    // Get and cache the initial size of the window
    m_size = m_impl->getSize();
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(VideoMode mode, const String& title, Style style, State state) :
WindowBase(priv::WindowImpl::create(mode,
                                    title,
                                    style,
                                    state,
                                    ContextSettings{/* depthBits */ 0,
                                                    /* stencilBits */ 0,
                                                    /* antialiasingLevel */ 0,
                                                    /* majorVersion */ 0,
                                                    /* minorVersion */ 0,
                                                    /* attributeFlags */ ContextSettings::Attribute{0xFFFFFFFFu},
                                                    /* sRgbCapable */ false}))
{
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(VideoMode mode, const String& title, State state) :
WindowBase(mode, title, Style::Default, state)
{
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(WindowHandle handle) : WindowBase(priv::WindowImpl::create(handle))
{
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(VideoMode mode, const char* title, Style style, State state) :
WindowBase(mode, String(title), style, state)
{
}


////////////////////////////////////////////////////////////
WindowBase::WindowBase(VideoMode mode, const char* title, State state) : WindowBase(mode, String(title), state)
{
}


////////////////////////////////////////////////////////////
WindowBase::~WindowBase() = default;


////////////////////////////////////////////////////////////
WindowBase::WindowBase(WindowBase&&) noexcept = default;


////////////////////////////////////////////////////////////
WindowBase& WindowBase::operator=(WindowBase&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<Event> WindowBase::pollEvent()
{
    return filterEvent(m_impl->pollEvent());
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowBase::waitEvent(Time timeout)
{
    return filterEvent(m_impl->waitEvent(timeout));
}


////////////////////////////////////////////////////////////
Vector2i WindowBase::getPosition() const
{
    return m_impl->getPosition();
}


////////////////////////////////////////////////////////////
void WindowBase::setPosition(Vector2i position)
{
    m_impl->setPosition(position);
}


////////////////////////////////////////////////////////////
Vector2u WindowBase::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
void WindowBase::setSize(Vector2u size)
{
    // Constrain requested size within minimum and maximum bounds
    const auto minimumSize = m_impl->getMinimumSize().valueOr(Vector2u());
    const auto maximumSize = m_impl->getMaximumSize().valueOr(Vector2u{UINT_MAX, UINT_MAX});

    const auto width  = base::clamp(size.x, minimumSize.x, maximumSize.x);
    const auto height = base::clamp(size.y, minimumSize.y, maximumSize.y);

    // Do nothing if requested size matches current size
    const Vector2u clampedSize(width, height);
    if (clampedSize == m_size)
        return;

    m_impl->setSize(clampedSize);

    // Cache the new size
    m_size = clampedSize;
}


////////////////////////////////////////////////////////////
void WindowBase::setMinimumSize(const Vector2u& minimumSize)
{
    [[maybe_unused]] const auto validateMinimumSize = [&]
    {
        if (!m_impl->getMaximumSize().hasValue())
            return true;

        return minimumSize.x <= m_impl->getMaximumSize()->x && minimumSize.y <= m_impl->getMaximumSize()->y;
    };

    SFML_BASE_ASSERT(validateMinimumSize() && "Minimum size cannot be bigger than the maximum size along either axis");

    m_impl->setMinimumSize(sf::base::makeOptional(minimumSize));
    setSize(getSize());
}


////////////////////////////////////////////////////////////
void WindowBase::setMinimumSize(const base::Optional<Vector2u>& minimumSize)
{
    if (minimumSize.hasValue())
        setMinimumSize(*minimumSize);
    else
    {
        m_impl->setMinimumSize(base::nullOpt);
        setSize(getSize());
    }
}


////////////////////////////////////////////////////////////
void WindowBase::setMaximumSize(const Vector2u& maximumSize)
{
    [[maybe_unused]] const auto validateMaximumSize = [&]
    {
        if (!!m_impl->getMinimumSize().hasValue())
            return true;

        return maximumSize.x >= m_impl->getMinimumSize()->x && maximumSize.y >= m_impl->getMinimumSize()->y;
    };

    SFML_BASE_ASSERT(validateMaximumSize() && "Maximum size cannot be smaller than the minimum size along either axis");

    m_impl->setMaximumSize(sf::base::makeOptional(maximumSize));
    setSize(getSize());
}


////////////////////////////////////////////////////////////
void WindowBase::setMaximumSize(const base::Optional<Vector2u>& maximumSize)
{
    if (maximumSize.hasValue())
        setMinimumSize(*maximumSize);
    else
    {
        m_impl->setMaximumSize(base::nullOpt);
        setSize(getSize());
    }
}


////////////////////////////////////////////////////////////
void WindowBase::setTitle(const String& title)
{
    m_impl->setTitle(title);
}


////////////////////////////////////////////////////////////
void WindowBase::setTitle(const char* title)
{
    setTitle(String(title));
}


////////////////////////////////////////////////////////////
void WindowBase::setIcon(Vector2u size, const std::uint8_t* pixels)
{
    m_impl->setIcon(size, pixels);
}


////////////////////////////////////////////////////////////
void WindowBase::setVisible(bool visible)
{
    m_impl->setVisible(visible);
}


////////////////////////////////////////////////////////////
void WindowBase::setMouseCursorVisible(bool visible)
{
    m_impl->setMouseCursorVisible(visible);
}


////////////////////////////////////////////////////////////
void WindowBase::setMouseCursorGrabbed(bool grabbed)
{
    m_impl->setMouseCursorGrabbed(grabbed);
}


////////////////////////////////////////////////////////////
void WindowBase::setMouseCursor(const Cursor& cursor)
{
    m_impl->setMouseCursor(cursor.getImpl());
}


////////////////////////////////////////////////////////////
void WindowBase::setKeyRepeatEnabled(bool enabled)
{
    m_impl->setKeyRepeatEnabled(enabled);
}


////////////////////////////////////////////////////////////
void WindowBase::setJoystickThreshold(float threshold)
{
    m_impl->setJoystickThreshold(threshold);
}


////////////////////////////////////////////////////////////
void WindowBase::requestFocus()
{
    m_impl->requestFocus();
}


////////////////////////////////////////////////////////////
bool WindowBase::hasFocus() const
{
    return m_impl->hasFocus();
}


////////////////////////////////////////////////////////////
WindowHandle WindowBase::getNativeHandle() const
{
    return m_impl->getNativeHandle();
}


////////////////////////////////////////////////////////////
bool WindowBase::createVulkanSurface(const Vulkan::VulkanSurfaceData& vulkanSurfaceData)
{
    return m_impl->createVulkanSurface(vulkanSurfaceData);
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowBase::filterEvent(base::Optional<Event> event)
{
    // Cache the new size if needed
    if (event.hasValue() && event->getIf<Event::Resized>())
        m_size = event->getIf<Event::Resized>()->size;

    return event;
}

} // namespace sf
