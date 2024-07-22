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
#include <SFML/Window/Event.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/JoystickImpl.hpp>
#include <SFML/Window/JoystickManager.hpp>
#include <SFML/Window/Sensor.hpp>
#include <SFML/Window/SensorImpl.hpp>
#include <SFML/Window/SensorManager.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowImpl.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/TimeChronoUtil.hpp>

#include <SFML/Base/EnumArray.hpp>
#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Math.hpp>
#include <SFML/Base/UniquePtr.hpp>

#include <chrono>
#include <queue>

#if defined(SFML_SYSTEM_WINDOWS)

#include <SFML/Window/Win32/WindowImplWin32.hpp>
using WindowImplType = sf::priv::WindowImplWin32;

#include <SFML/Window/VulkanImpl.hpp>

#elif defined(SFML_SYSTEM_LINUX) || defined(SFML_SYSTEM_FREEBSD) || defined(SFML_SYSTEM_OPENBSD) || \
    defined(SFML_SYSTEM_NETBSD)

#if defined(SFML_USE_DRM)

#include <SFML/Window/DRM/WindowImplDRM.hpp>
using WindowImplType = sf::priv::WindowImplDRM;

#define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

#else

#include <SFML/Window/Unix/WindowImplX11.hpp>
using WindowImplType = sf::priv::WindowImplX11;

#include <SFML/Window/VulkanImpl.hpp>

#endif

#elif defined(SFML_SYSTEM_MACOS)

#include <SFML/Window/macOS/WindowImplCocoa.hpp>
using WindowImplType = sf::priv::WindowImplCocoa;

#define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

#elif defined(SFML_SYSTEM_IOS)

#include <SFML/Window/iOS/WindowImplUIKit.hpp>
using WindowImplType = sf::priv::WindowImplUIKit;

#define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

#elif defined(SFML_SYSTEM_ANDROID)

#include <SFML/Window/Android/WindowImplAndroid.hpp>
using WindowImplType = sf::priv::WindowImplAndroid;

#define SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

#endif


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
// Yes, this is a rather weird namespace.
namespace WindowImplImpl
{
const sf::priv::WindowImpl* fullscreenWindow = nullptr;
} // namespace WindowImplImpl
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct WindowImpl::JoystickStatesImpl
{
    JoystickState states[Joystick::Count]{}; //!< Previous state of the joysticks
};


////////////////////////////////////////////////////////////
struct WindowImpl::Impl
{
    std::queue<Event>                   events;                         //!< Queue of available events
    base::UniquePtr<JoystickStatesImpl> joystickStatesImpl;             //!< Previous state of the joysticks (PImpl)
    base::EnumArray<Sensor::Type, Vector3f, Sensor::Count> sensorValue; //!< Previous value of the sensors
    float joystickThreshold{0.1f}; //!< Joystick threshold (minimum motion for "move" event to be generated)
    base::EnumArray<Joystick::Axis, float, Joystick::AxisCount>
        previousAxes[Joystick::Count]{}; //!< Position of each axis last time a move event triggered, in range [-100, 100]
    base::Optional<Vector2u> minimumSize; //!< Minimum window size
    base::Optional<Vector2u> maximumSize; //!< Maximum window size

    explicit Impl(base::UniquePtr<JoystickStatesImpl>&& theJoystickStatesImpl) :
    joystickStatesImpl(SFML_BASE_MOVE(theJoystickStatesImpl))
    {
    }
};


////////////////////////////////////////////////////////////
base::UniquePtr<WindowImpl> WindowImpl::create(VideoMode mode, const String& title, Style style, State state, const ContextSettings& settings)
{
    // Fullscreen style requires some tests
    if (state == State::Fullscreen)
    {
        // Make sure there's not already a fullscreen window (only one is allowed)
        if (WindowImplImpl::fullscreenWindow != nullptr)
        {
            err() << "Creating two fullscreen windows is not allowed, switching to windowed mode";
            state = State::Windowed;
        }
        else
        {
            // Make sure that the chosen video mode is compatible
            if (!mode.isValid())
            {
                err() << "The requested video mode is not available, switching to a valid mode";

                SFML_BASE_ASSERT(!VideoMode::getFullscreenModes().empty() && "No video modes available");
                mode = VideoMode::getFullscreenModes()[0];

                err() << "  VideoMode: { size: { " << mode.size.x << ", " << mode.size.y
                      << " }, bitsPerPixel: " << mode.bitsPerPixel << " }";
            }
        }
    }

// Check validity of style according to the underlying platform
#if defined(SFML_SYSTEM_IOS) || defined(SFML_SYSTEM_ANDROID)
    if (state == State::Fullscreen)
        style &= ~static_cast<std::uint32_t>(Style::Titlebar);
    else
        style |= Style::Titlebar;
#else
    if (!!(style & Style::Close) || !!(style & Style::Resize))
        style |= Style::Titlebar;
#endif

    auto windowImpl = base::makeUnique<WindowImplType>(mode, title, style, state, settings);

    if (state == State::Fullscreen)
        WindowImplImpl::fullscreenWindow = windowImpl.get();

    return windowImpl;
}


////////////////////////////////////////////////////////////
base::UniquePtr<WindowImpl> WindowImpl::create(WindowHandle handle)
{
    return base::makeUnique<WindowImplType>(handle);
}


////////////////////////////////////////////////////////////
WindowImpl::WindowImpl() : m_impl(base::makeUnique<JoystickStatesImpl>())
{
    // Get the initial joystick states
    JoystickManager::getInstance().update();
    for (unsigned int i = 0; i < Joystick::Count; ++i)
    {
        m_impl->joystickStatesImpl->states[i] = JoystickManager::getInstance().getState(i);
        m_impl->previousAxes[i].fill(0.f);
    }

    // Get the initial sensor states
    for (Vector3f& vec : m_impl->sensorValue.data)
        vec = Vector3f(0, 0, 0);
}


////////////////////////////////////////////////////////////
WindowImpl::~WindowImpl()
{
    if (WindowImplImpl::fullscreenWindow == this)
        WindowImplImpl::fullscreenWindow = nullptr;
}


////////////////////////////////////////////////////////////
base::Optional<Vector2u> WindowImpl::getMinimumSize() const
{
    return m_impl->minimumSize;
}


////////////////////////////////////////////////////////////
base::Optional<Vector2u> WindowImpl::getMaximumSize() const
{
    return m_impl->maximumSize;
}


////////////////////////////////////////////////////////////
void WindowImpl::setJoystickThreshold(float threshold)
{
    m_impl->joystickThreshold = threshold;
}


////////////////////////////////////////////////////////////
void WindowImpl::setMinimumSize(const base::Optional<Vector2u>& minimumSize)
{
    m_impl->minimumSize = minimumSize;
}


////////////////////////////////////////////////////////////
void WindowImpl::setMaximumSize(const base::Optional<Vector2u>& maximumSize)
{
    m_impl->maximumSize = maximumSize;
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowImpl::waitEvent(Time timeout)
{
    const auto timedOut = [&, startTime = std::chrono::steady_clock::now()]
    {
        const bool infiniteTimeout = timeout == Time::Zero;
        return !infiniteTimeout && (std::chrono::steady_clock::now() - startTime) >= TimeChronoUtil::toDuration(timeout);
    };

    // If the event queue is empty, let's first check if new events are available from the OS
    if (m_impl->events.empty())
        populateEventQueue();

    // Here we use a manual wait loop instead of the optimized wait-event provided by the OS,
    // so that we don't skip joystick events (which require polling)
    while (m_impl->events.empty() && !timedOut())
    {
        sleep(milliseconds(10));
        populateEventQueue();
    }

    return popEvent();
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowImpl::pollEvent()
{
    // If the event queue is empty, let's first check if new events are available from the OS
    if (m_impl->events.empty())
        populateEventQueue();

    return popEvent();
}


////////////////////////////////////////////////////////////
base::Optional<Event> WindowImpl::popEvent()
{
    base::Optional<Event> event; // Use a single local variable for NRVO

    if (!m_impl->events.empty())
    {
        event.emplace(m_impl->events.front());
        m_impl->events.pop();
    }

    return event;
}


////////////////////////////////////////////////////////////
void WindowImpl::pushEvent(const Event& event)
{
    m_impl->events.push(event);
}


////////////////////////////////////////////////////////////
void WindowImpl::processJoystickEvents()
{
    // First update the global joystick states
    JoystickManager::getInstance().update();

    for (unsigned int i = 0; i < Joystick::Count; ++i)
    {
        // Copy the previous state of the joystick and get the new one
        const JoystickState previousState     = m_impl->joystickStatesImpl->states[i];
        m_impl->joystickStatesImpl->states[i] = JoystickManager::getInstance().getState(i);

        // Connection state
        const bool connected = m_impl->joystickStatesImpl->states[i].connected;
        if (previousState.connected ^ connected)
        {
            if (connected)
                pushEvent(Event::JoystickConnected{i});
            else
                pushEvent(Event::JoystickDisconnected{i});

            // Clear previous axes positions
            if (connected)
                m_impl->previousAxes[i].fill(0.f);
        }

        if (!connected)
            continue;

        const JoystickCaps caps = JoystickManager::getInstance().getCapabilities(i);

        // Axes
        for (unsigned int j = 0; j < Joystick::AxisCount; ++j)
        {
            const auto axis = static_cast<Joystick::Axis>(j);
            if (!caps.axes[axis])
                continue;

            const float prevPos = m_impl->previousAxes[i][axis];
            const float currPos = m_impl->joystickStatesImpl->states[i].axes[axis];
            if (base::fabs(currPos - prevPos) >= m_impl->joystickThreshold)
            {
                pushEvent(Event::JoystickMoved{i, axis, currPos});
                m_impl->previousAxes[i][axis] = currPos;
            }
        }

        // Buttons
        for (unsigned int j = 0; j < caps.buttonCount; ++j)
        {
            const bool prevPressed = previousState.buttons[j];
            const bool currPressed = m_impl->joystickStatesImpl->states[i].buttons[j];

            if (prevPressed ^ currPressed)
            {
                if (currPressed)
                    pushEvent(Event::JoystickButtonPressed{i, j});
                else
                    pushEvent(Event::JoystickButtonReleased{i, j});
            }
        }
    }
}


////////////////////////////////////////////////////////////
void WindowImpl::processSensorEvents()
{
    // First update the sensor states
    SensorManager::getInstance().update();

    for (unsigned int i = 0; i < Sensor::Count; ++i)
    {
        const auto sensor = static_cast<Sensor::Type>(i);

        // Only process enabled sensors
        if (!SensorManager::getInstance().isEnabled(sensor))
            continue;

        // Copy the previous value of the sensor and get the new one
        const Vector3f previousValue = m_impl->sensorValue[sensor];
        m_impl->sensorValue[sensor]  = SensorManager::getInstance().getValue(sensor);

        // If the value has changed, trigger an event
        if (m_impl->sensorValue[sensor] != previousValue) // TODO P2: use a threshold?
            pushEvent(Event::SensorChanged{sensor, m_impl->sensorValue[sensor]});
    }
}


////////////////////////////////////////////////////////////
void WindowImpl::populateEventQueue()
{
    processJoystickEvents();
    processSensorEvents();
    processEvents();
}


////////////////////////////////////////////////////////////
bool WindowImpl::createVulkanSurface([[maybe_unused]] const Vulkan::VulkanSurfaceData& vulkanSurfaceData) const
{
#ifdef SFML_VULKAN_IMPLEMENTATION_NOT_AVAILABLE

    return false;

#else

    return VulkanImpl::createVulkanSurface(vulkanSurfaceData.instance,
                                           getNativeHandle(),
                                           vulkanSurfaceData.surface,
                                           vulkanSurfaceData.allocator);

#endif
}

} // namespace sf::priv
