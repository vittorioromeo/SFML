#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Base/InPlacePImpl.hpp>


namespace sf::Joystick
{
struct Identification;
} // namespace sf::Joystick

namespace sf::priv
{
struct JoystickCaps;
struct JoystickState;

////////////////////////////////////////////////////////////
/// \brief Global joystick manager
///
////////////////////////////////////////////////////////////
class JoystickManager
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    JoystickManager(const JoystickManager&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    JoystickManager& operator=(const JoystickManager&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Get the global unique instance of the manager
    ///
    /// \return Unique instance of the joystick manager
    ///
    ////////////////////////////////////////////////////////////
    static JoystickManager& getInstance();

    ////////////////////////////////////////////////////////////
    /// \brief Get the capabilities for an open joystick
    ///
    /// \param joystick Index of the joystick
    ///
    /// \return Capabilities of the joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const JoystickCaps& getCapabilities(unsigned int joystick) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current state of an open joystick
    ///
    /// \param joystick Index of the joystick
    ///
    /// \return Current state of the joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const JoystickState& getState(unsigned int joystick) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the identification for an open joystick
    ///
    /// \param joystick Index of the joystick
    ///
    /// \return Identification for the joystick
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const Joystick::Identification& getIdentification(unsigned int joystick) const;

    ////////////////////////////////////////////////////////////
    /// \brief Update the state of all the joysticks
    ///
    ////////////////////////////////////////////////////////////
    void update();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    JoystickManager();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~JoystickManager();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 12288> m_impl; //!< Implementation details
};

} // namespace sf::priv
