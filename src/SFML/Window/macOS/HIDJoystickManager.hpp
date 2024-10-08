#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDManager.h>

namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief `sf::priv::InputImpl` helper
///
/// This class manage as a singleton instance the joysticks.
/// It's only purpose is to help `sf::priv::JoystickImpl` class.
///
////////////////////////////////////////////////////////////
class HIDJoystickManager
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    HIDJoystickManager(const HIDJoystickManager&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    HIDJoystickManager& operator=(const HIDJoystickManager&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Get the unique instance of the class
    ///
    /// \note Private use only
    ///
    /// \return Reference to the HIDJoystickManager instance
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static HIDJoystickManager& getInstance();

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of currently connected joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getJoystickCount();

    ////////////////////////////////////////////////////////////
    /// \brief Copy the devices associated with this HID manager
    ///
    /// \return a retained CFSetRef of IOHIDDeviceRef or a null pointer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] CFSetRef copyJoysticks();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    HIDJoystickManager();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~HIDJoystickManager();

    ////////////////////////////////////////////////////////////
    /// \brief Make sure all event have been processed in the run loop
    ///
    ////////////////////////////////////////////////////////////
    void update();

    ////////////////////////////////////////////////////////////
    /// \brief Private "plug-in" callback
    /// \note Only 'context' parameter is used.
    /// \see IOHIDDeviceCallback
    ///
    ////////////////////////////////////////////////////////////
    static void pluggedIn(void* context, IOReturn, void*, IOHIDDeviceRef);

    ////////////////////////////////////////////////////////////
    /// \brief Private "plug-out" callback
    /// \note Only 'context' parameter is used.
    /// \see IOHIDDeviceCallback
    ///
    ////////////////////////////////////////////////////////////
    static void pluggedOut(void* context, IOReturn, void*, IOHIDDeviceRef);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    IOHIDManagerRef m_manager{};       ///< HID Manager
    unsigned int    m_joystickCount{}; ///< Number of joysticks currently connected
};


} // namespace sf::priv
