#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/JoystickIdentification.hpp>

#include <Doctest.hpp>

#include <SystemUtil.hpp>

TEST_CASE("[Window] sf::Joystick")
{
    SECTION("Constants")
    {
        STATIC_CHECK(sf::Joystick::Count == 8);
        STATIC_CHECK(sf::Joystick::ButtonCount == 32);
        STATIC_CHECK(sf::Joystick::AxisCount == 8);
    }

    SECTION("Identification")
    {
        const sf::Joystick::Identification identification;
        CHECK(identification.name == "No Joystick");
        CHECK(identification.vendorId == 0);
        CHECK(identification.productId == 0);
    }

    // By avoiding calling sf::Joystick::update() we can guarantee that
    // no joysticks will be detected. This is how we can ensure these
    // tests are portable and reliable.

    for (unsigned int joystick = 0u; joystick < sf::Joystick::Count; ++joystick)
    {

        SECTION("isConnected()")
        {
            CHECK(!sf::Joystick::isConnected(joystick));
        }

        SECTION("getButtonCount()")
        {
            CHECK(sf::Joystick::getButtonCount(joystick) == 0);
        }

        SECTION("hasAxis()")
        {
            CHECK(!sf::Joystick::hasAxis(joystick, sf::Joystick::Axis::X));
            CHECK(!sf::Joystick::hasAxis(joystick, sf::Joystick::Axis::Y));
            CHECK(!sf::Joystick::hasAxis(joystick, sf::Joystick::Axis::Z));
            CHECK(!sf::Joystick::hasAxis(joystick, sf::Joystick::Axis::R));
            CHECK(!sf::Joystick::hasAxis(joystick, sf::Joystick::Axis::U));
            CHECK(!sf::Joystick::hasAxis(joystick, sf::Joystick::Axis::V));
            CHECK(!sf::Joystick::hasAxis(joystick, sf::Joystick::Axis::PovX));
            CHECK(!sf::Joystick::hasAxis(joystick, sf::Joystick::Axis::PovY));
        }

        SECTION("isButtonPressed()")
        {
            for (unsigned int button = 0u; button < sf::Joystick::ButtonCount; ++button)
                CHECK(!sf::Joystick::isButtonPressed(joystick, button));
        }

        SECTION("getAxisPosition")
        {
            CHECK(sf::Joystick::getAxisPosition(joystick, sf::Joystick::Axis::X) == 0);
            CHECK(sf::Joystick::getAxisPosition(joystick, sf::Joystick::Axis::Y) == 0);
            CHECK(sf::Joystick::getAxisPosition(joystick, sf::Joystick::Axis::Z) == 0);
            CHECK(sf::Joystick::getAxisPosition(joystick, sf::Joystick::Axis::R) == 0);
            CHECK(sf::Joystick::getAxisPosition(joystick, sf::Joystick::Axis::U) == 0);
            CHECK(sf::Joystick::getAxisPosition(joystick, sf::Joystick::Axis::V) == 0);
            CHECK(sf::Joystick::getAxisPosition(joystick, sf::Joystick::Axis::PovX) == 0);
            CHECK(sf::Joystick::getAxisPosition(joystick, sf::Joystick::Axis::PovY) == 0);
        }

        SECTION("getIdentification()")
        {
            const auto identification = sf::Joystick::getIdentification(joystick);
            CHECK(identification.name == "No Joystick");
            CHECK(identification.vendorId == 0);
            CHECK(identification.productId == 0);
        }
    }
}
