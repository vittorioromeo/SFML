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

#include <SFML/Window/JoystickAxis.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Sensor.hpp>

#include <SFML/System/Vector2.hpp>

#include <SFML/Base/Variant.hpp>

#include <cstdint>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Defines a system event and its parameters
///
////////////////////////////////////////////////////////////
class SFML_WINDOW_API Event
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Closed event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct Closed
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Resized event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct Resized
    {
        Vector2u size; //!< New size, in pixels
    };

    ////////////////////////////////////////////////////////////
    /// \brief Lost focus event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct FocusLost
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Gained focus event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct FocusGained
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Text event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct TextEntered
    {
        std::uint32_t unicode{}; //!< UTF-32 Unicode value of the character
    };

    ////////////////////////////////////////////////////////////
    /// \brief Key pressed event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct KeyPressed
    {
        Keyboard::Key      code{};     //!< Code of the key that has been pressed
        Keyboard::Scancode scancode{}; //!< Physical code of the key that has been pressed
        bool               alt{};      //!< Is the Alt key pressed?
        bool               control{};  //!< Is the Control key pressed?
        bool               shift{};    //!< Is the Shift key pressed?
        bool               system{};   //!< Is the System key pressed?
    };

    ////////////////////////////////////////////////////////////
    /// \brief Key released event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct KeyReleased
    {
        Keyboard::Key      code{};     //!< Code of the key that has been released
        Keyboard::Scancode scancode{}; //!< Physical code of the key that has been released
        bool               alt{};      //!< Is the Alt key pressed?
        bool               control{};  //!< Is the Control key pressed?
        bool               shift{};    //!< Is the Shift key pressed?
        bool               system{};   //!< Is the System key pressed?
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse wheel scrolled event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct MouseWheelScrolled
    {
        Mouse::Wheel wheel{}; //!< Which wheel (for mice with multiple ones)
        float delta{}; //!< Wheel offset (positive is up/left, negative is down/right). High-precision mice may use non-integral offsets.
        Vector2i position; //!< Position of the mouse pointer, relative to the top left of the owner window
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse button pressed event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct MouseButtonPressed
    {
        Mouse::Button button{}; //!< Code of the button that has been pressed
        Vector2i      position; //!< Position of the mouse pointer, relative to the top left of the owner window
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse button released event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct MouseButtonReleased
    {
        Mouse::Button button{}; //!< Code of the button that has been released
        Vector2i      position; //!< Position of the mouse pointer, relative to the top left of the owner window
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse move event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct MouseMoved
    {
        Vector2i position; //!< Position of the mouse pointer, relative to the top left of the owner window
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse move raw event subtype
    ///
    /// Raw mouse input data comes unprocessed from the
    /// operating system hence "raw". While the MouseMoved
    /// position value is dependent on the screen resolution,
    /// raw data is not. If the physical mouse is moved too
    /// little to cause the screen cursor to move at least a
    /// single pixel, no MouseMoved event will be generated. In
    /// contrast, any movement information generated by the
    /// mouse independent of its sensor resolution will always
    /// generate a MouseMovedRaw event.
    ///
    /// In addition to screen resolution independence, raw
    /// mouse data also does not have mouse acceleration or
    /// smoothing applied to it as MouseMoved does.
    ///
    /// Raw mouse movement data is intended for controlling
    /// non-cursor movement, e.g. controlling the camera
    /// orientation in a first person view, whereas MouseMoved
    /// is intended primarily for controlling things related to
    /// the screen cursor hence the additional processing
    /// applied to it.
    ///
    /// Currently, raw mouse input events will only be generated
    /// on Windows and Linux.
    ///
    ////////////////////////////////////////////////////////////
    struct MouseMovedRaw
    {
        Vector2i delta; ///< Delta movement of the mouse since the last event
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse entered event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct MouseEntered
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Mouse left event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct MouseLeft
    {
    };

    ////////////////////////////////////////////////////////////
    /// \brief Joystick button pressed event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct JoystickButtonPressed
    {
        unsigned int joystickId{}; //!< Index of the joystick (in range [0 .. Joystick::Count - 1])
        unsigned int button{}; //!< Index of the button that has been pressed (in range [0 .. Joystick::ButtonCount - 1])
    };

    ////////////////////////////////////////////////////////////
    /// \brief Joystick button released event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct JoystickButtonReleased
    {
        unsigned int joystickId{}; //!< Index of the joystick (in range [0 .. Joystick::Count - 1])
        unsigned int button{}; //!< Index of the button that has been released (in range [0 .. Joystick::ButtonCount - 1])
    };

    ////////////////////////////////////////////////////////////
    /// \brief Joystick axis move event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct JoystickMoved
    {
        unsigned int   joystickId{}; //!< Index of the joystick (in range [0 .. Joystick::Count - 1])
        Joystick::Axis axis{};       //!< Axis on which the joystick moved
        float          position{};   //!< New position on the axis (in range [-100 .. 100])
    };

    ////////////////////////////////////////////////////////////
    /// \brief Joystick connected event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct JoystickConnected
    {
        unsigned int joystickId{}; //!< Index of the joystick (in range [0 .. Joystick::Count - 1])
    };

    ////////////////////////////////////////////////////////////
    /// \brief Joystick disconnected event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct JoystickDisconnected
    {
        unsigned int joystickId{}; //!< Index of the joystick (in range [0 .. Joystick::Count - 1])
    };

    ////////////////////////////////////////////////////////////
    /// \brief Touch began event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct TouchBegan
    {
        unsigned int finger{}; //!< Index of the finger in case of multi-touch events
        Vector2i     position; //!< Start position of the touch, relative to the top left of the owner window
    };

    ////////////////////////////////////////////////////////////
    /// \brief Touch moved event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct TouchMoved
    {
        unsigned int finger{}; //!< Index of the finger in case of multi-touch events
        Vector2i     position; //!< Current position of the touch, relative to the top left of the owner window
    };

    ////////////////////////////////////////////////////////////
    /// \brief Touch ended event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct TouchEnded
    {
        unsigned int finger{}; //!< Index of the finger in case of multi-touch events
        Vector2i     position; //!< Final position of the touch, relative to the top left of the owner window
    };

    ////////////////////////////////////////////////////////////
    /// \brief Sensor event subtype
    ///
    ////////////////////////////////////////////////////////////
    struct SensorChanged
    {
        Sensor::Type type{}; //!< Type of the sensor
        Vector3f     value;  //!< Current value of the sensor on the X, Y, and Z axes
    };

    ////////////////////////////////////////////////////////////
    /// \brief Deleted default constructor
    ///
    ////////////////////////////////////////////////////////////
    Event() = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Construct from a given `sf::Event` subtype
    ///
    /// \tparam TEventSubtype Type of event subtype used to construct the event
    ///
    /// \param eventSubtype Event subtype instance used to construct the event
    ///
    ////////////////////////////////////////////////////////////
    template <typename TEventSubtype>
    [[nodiscard]] Event(const TEventSubtype& eventSubtype);

    ////////////////////////////////////////////////////////////
    /// \brief Check current event subtype
    ///
    /// \tparam TEventSubtype Type of the event subtype to check against
    ///
    /// \return True if the current event subtype matches given template parameter
    ///
    ////////////////////////////////////////////////////////////
    template <typename TEventSubtype>
    [[nodiscard]] bool is() const;

    ////////////////////////////////////////////////////////////
    /// \brief Attempt to get specified event subtype
    ///
    /// \tparam TEventSubtype Type of the desired event subtype
    ///
    /// \return Address of current event subtype, otherwise `nullptr`
    ///
    ////////////////////////////////////////////////////////////
    template <typename TEventSubtype>
    [[nodiscard]] const TEventSubtype* getIf() const;

    ////////////////////////////////////////////////////////////
    /// \brief Applies the specified `visitor` to the event
    ///
    /// \return Transparently forwards whatever `visitor` returns
    ///
    ////////////////////////////////////////////////////////////
    template <typename Visitor>
    decltype(auto) visit(Visitor&& visitor) const;

    ////////////////////////////////////////////////////////////
    /// \brief Invokes `visit` with an overload created from `handlers...`
    ///
    /// \return Transparently forwards whatever `visit` returns
    ///
    ////////////////////////////////////////////////////////////
    template <typename... Handlers>
    decltype(auto) match(Handlers&&... handlers) const;

private:
    // clang-format off

    #define SFML_PRIV_EVENTS_X_MACRO(x, xSep)         \
        x(::sf::Event::Closed)                 xSep() \
        x(::sf::Event::Resized)                xSep() \
        x(::sf::Event::FocusLost)              xSep() \
        x(::sf::Event::FocusGained)            xSep() \
        x(::sf::Event::TextEntered)            xSep() \
        x(::sf::Event::KeyPressed)             xSep() \
        x(::sf::Event::KeyReleased)            xSep() \
        x(::sf::Event::MouseWheelScrolled)     xSep() \
        x(::sf::Event::MouseButtonPressed)     xSep() \
        x(::sf::Event::MouseButtonReleased)    xSep() \
        x(::sf::Event::MouseMoved)             xSep() \
        x(::sf::Event::MouseMovedRaw)          xSep() \
        x(::sf::Event::MouseEntered)           xSep() \
        x(::sf::Event::MouseLeft)              xSep() \
        x(::sf::Event::JoystickButtonPressed)  xSep() \
        x(::sf::Event::JoystickButtonReleased) xSep() \
        x(::sf::Event::JoystickMoved)          xSep() \
        x(::sf::Event::JoystickConnected)      xSep() \
        x(::sf::Event::JoystickDisconnected)   xSep() \
        x(::sf::Event::TouchBegan)             xSep() \
        x(::sf::Event::TouchMoved)             xSep() \
        x(::sf::Event::TouchEnded)             xSep() \
        x(::sf::Event::SensorChanged)

    // clang-format on

#define SFML_PRIV_EVENT_X_EXPAND(x) x
#define SFML_PRIV_EVENT_X_COMMA()   ,

#define SFML_PRIV_EVENT_VARIANT_TYPE \
    ::sfvr::tinyvariant<SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_EXPAND, SFML_PRIV_EVENT_X_COMMA)>

    using VariantType = SFML_PRIV_EVENT_VARIANT_TYPE;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    VariantType m_data; //!< Event data
};

} // namespace sf


////////////////////////////////////////////////////////////
// Explicit instantiation declarations
////////////////////////////////////////////////////////////

extern template class SFML_PRIV_EVENT_VARIANT_TYPE;

#define SFML_PRIV_EVENT_X_EXTERN_TEMPLATE_CTOR(x)  extern template sf::Event::Event(const x&);
#define SFML_PRIV_EVENT_X_EXTERN_TEMPLATE_IS(x)    extern template bool sf::Event::is<x>() const;
#define SFML_PRIV_EVENT_X_EXTERN_TEMPLATE_GETIF(x) extern template const x* sf::Event::getIf<x>() const;

#define SFML_PRIV_EVENT_X_SEMICOLON() ;

SFML_PRIV_EVENTS_X_MACRO(SFML_PRIV_EVENT_X_EXTERN_TEMPLATE_GETIF, SFML_PRIV_EVENT_X_SEMICOLON);

#include <SFML/Window/Event.inl>

////////////////////////////////////////////////////////////
/// \class sf::Event
/// \ingroup window
///
/// `sf::Event` holds all the information about a system event
/// that just happened. Events are  using the
/// `sf::Window::pollEvent` and `sf::Window::waitEvent` functions.
///
/// A `sf::Event` instance contains the subtype of the event
/// (mouse moved, key pressed, window closed, ...) as well
/// as the details about this particular event. Each event
/// corresponds to a different subtype struct which contains
/// the data required to process that event.
///
/// Event subtype are event types belonging to `sf::Event`,
/// such as `sf::Event::Closed` or `sf::Event::MouseMoved`.
///
/// The way to access the current active event subtype is via
/// `sf::Event::getIf`. This member function returns the address
/// of the event subtype struct if the event subtype matches the
/// active event, otherwise it returns `nullptr`.
///
/// `sf::Event::is` is used to check the active event subtype
/// without actually reading any of the corresponding event data.
///
/// \code
/// while (const sf::base::Optional event = window.pollEvent())
/// {
///     // Window closed or escape key pressed: exit
///     if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
///         return 0; // break out of both event and main loops
///
///     // The window was resized
///     if (const auto* resized = event->getIf<sf::Event::Resized>())
///         doSomethingWithTheNewSize(resized->size);
///
///     // etc ...
/// }
/// \endcode
///
////////////////////////////////////////////////////////////
