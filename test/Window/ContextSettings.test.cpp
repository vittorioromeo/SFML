#include <SFML/Window/ContextSettings.hpp>

#include <Doctest.hpp>

#include <type_traits>

TEST_CASE("[Window] sf::ContextSettings")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(std::is_copy_constructible_v<sf::ContextSettings>);
        STATIC_CHECK(std::is_copy_assignable_v<sf::ContextSettings>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::ContextSettings>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::ContextSettings>);
    }

    SECTION("Construction")
    {
        SECTION("Aggregate initialization -- Nothing")
        {
            constexpr sf::ContextSettings contextSettings;
            STATIC_CHECK(contextSettings.depthBits == 0);
            STATIC_CHECK(contextSettings.stencilBits == 0);
            STATIC_CHECK(contextSettings.antialiasingLevel == 0);
            STATIC_CHECK(contextSettings.majorVersion == 1);
            STATIC_CHECK(contextSettings.minorVersion == 1);
#ifndef SFML_OPENGL_ES
            STATIC_CHECK(contextSettings.attributeFlags == sf::ContextSettings::Attribute::Default);
#else
            STATIC_CHECK(contextSettings.attributeFlags == sf::ContextSettings::Attribute::Core);
#endif
            STATIC_CHECK(contextSettings.sRgbCapable == false);
        }

        SECTION("Aggregate initialization -- Everything")
        {
            constexpr sf::ContextSettings contextSettings{1, 1, 2, 3, 5, sf::ContextSettings::Attribute::Core, true};
            STATIC_CHECK(contextSettings.depthBits == 1);
            STATIC_CHECK(contextSettings.stencilBits == 1);
            STATIC_CHECK(contextSettings.antialiasingLevel == 2);
            STATIC_CHECK(contextSettings.majorVersion == 3);
            STATIC_CHECK(contextSettings.minorVersion == 5);
            STATIC_CHECK(contextSettings.attributeFlags == sf::ContextSettings::Attribute::Core);
            STATIC_CHECK(contextSettings.sRgbCapable == true);
        }
    }
}
