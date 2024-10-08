#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/JoystickImpl.hpp"

#include "SFML/Base/InPlacePImpl.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf::priv
{
struct JoystickIdentification;
} // namespace sf::priv


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Emscripten implementation of joysticks
///
////////////////////////////////////////////////////////////
class JoystickImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    explicit JoystickImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~JoystickImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Perform the global initialization of the joystick module
    ///
    ////////////////////////////////////////////////////////////
    static void initialize();

    ////////////////////////////////////////////////////////////
    /// \brief Perform the global cleanup of the joystick module
    ///
    ////////////////////////////////////////////////////////////
    static void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Check if a joystick is currently connected
    ///
    /// \param index Index of the joystick to check
    ///
    /// \return `true` if the joystick is connected, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    static bool isConnected(unsigned int index);

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable lazy enumeration updates
    ///
    /// \param status Whether to rely on windows triggering enumeration updates
    ///
    ////////////////////////////////////////////////////////////
    static void setLazyUpdates(bool status);

    ////////////////////////////////////////////////////////////
    /// \brief Update the connection status of all joysticks
    ///
    ////////////////////////////////////////////////////////////
    static void updateConnections();

    ////////////////////////////////////////////////////////////
    /// \brief Open the joystick
    ///
    /// \param index Index assigned to the joystick
    ///
    /// \return `true` on success, `false` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool open(unsigned int index);

    ////////////////////////////////////////////////////////////
    /// \brief Close the joystick
    ///
    ////////////////////////////////////////////////////////////
    void close();

    ////////////////////////////////////////////////////////////
    /// \brief Get the joystick capabilities
    ///
    /// \return Joystick capabilities
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] JoystickCapabilities getCapabilities() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the joystick identification
    ///
    /// \return Joystick identification
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const JoystickIdentification& getIdentification() const;

    ////////////////////////////////////////////////////////////
    /// \brief Update the joystick and get its new state
    ///
    /// \return Joystick state
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] JoystickState update();

    ////////////////////////////////////////////////////////////
    /// \brief Perform the global initialization of the joystick module (DInput)
    ///
    ////////////////////////////////////////////////////////////
    static void initializeDInput();

    ////////////////////////////////////////////////////////////
    /// \brief Perform the global cleanup of the joystick module (DInput)
    ///
    ////////////////////////////////////////////////////////////
    static void cleanupDInput();

    ////////////////////////////////////////////////////////////
    /// \brief Check if a joystick is currently connected (DInput)
    ///
    /// \param index Index of the joystick to check
    ///
    /// \return `true` if the joystick is connected, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    static bool isConnectedDInput(unsigned int index);

    ////////////////////////////////////////////////////////////
    /// \brief Update the connection status of all joysticks (DInput)
    ///
    ////////////////////////////////////////////////////////////
    static void updateConnectionsDInput();

    ////////////////////////////////////////////////////////////
    /// \brief Open the joystick (DInput)
    ///
    /// \param index Index assigned to the joystick
    ///
    /// \return `true` on success, `false` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool openDInput(unsigned int index);

    ////////////////////////////////////////////////////////////
    /// \brief Close the joystick (DInput)
    ///
    ////////////////////////////////////////////////////////////
    void closeDInput();

    ////////////////////////////////////////////////////////////
    /// \brief Update the joystick and get its new state (DInput, Buffered)
    ///
    /// \return Joystick state
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] JoystickState updateDInputBuffered();

    ////////////////////////////////////////////////////////////
    /// \brief Update the joystick and get its new state (DInput, Polled)
    ///
    /// \return Joystick state
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] JoystickState updateDInputPolled();

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 1280> m_impl; //!< Implementation details
};

} // namespace sf::priv
