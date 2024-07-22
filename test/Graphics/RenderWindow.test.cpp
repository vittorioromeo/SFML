#include <SFML/Graphics/RenderWindow.hpp>

// Other 1st party headers
#include "C:/OHWorkspace/SFML/extlibs/headers/glad/include/glad/gl.h"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>

#include <SFML/Window/GraphicsContext.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/String.hpp>

#include <Doctest.hpp>

#include <GraphicsUtil.hpp>
#include <WindowUtil.hpp>

#include <type_traits>


TEST_CASE("[Graphics] sf::RenderWindow" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(std::has_virtual_destructor_v<sf::RenderWindow>); // because of RenderTarget, not WindowBase
        STATIC_CHECK(!std::is_copy_constructible_v<sf::RenderWindow>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::RenderWindow>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::RenderWindow>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::RenderWindow>);
    }

    SECTION("Construction")
    {
        SECTION("Style, state, and settings")
        {
            const sf::RenderWindow window(graphicsContext,
                                          sf::VideoMode(sf::Vector2u{256, 256}, 24),
                                          "Window Title",
                                          sf::Style::Default,
                                          sf::State::Windowed,
                                          sf::ContextSettings{});
            CHECK(window.getSize() == sf::Vector2u{256, 256});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
            CHECK(!window.isSrgb());
            CHECK(window.getView().getCenter() == sf::Vector2f{128, 128});
            CHECK(window.getView().getSize() == sf::Vector2f{256, 256});
            CHECK(window.getView().getRotation() == sf::Angle::Zero);
            CHECK(window.getView().getViewport() == sf::FloatRect({0, 0}, {1, 1}));
            CHECK(window.getView().getTransform() == sf::Transform(0.0078125f, 0, -1, 0, -0.0078125f, 1, 0, 0, 1));
        }

        SECTION("State and settings")
        {
            const sf::RenderWindow window(graphicsContext,
                                          sf::VideoMode(sf::Vector2u{240, 300}, 24),
                                          "Window Title",
                                          sf::State::Windowed,
                                          sf::ContextSettings{});
            CHECK(window.getSize() == sf::Vector2u{240, 300});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
            CHECK(!window.isSrgb());
            CHECK(window.getView().getCenter() == sf::Vector2f{120, 150});
            CHECK(window.getView().getSize() == sf::Vector2f{240, 300});
            CHECK(window.getView().getRotation() == sf::Angle::Zero);
            CHECK(window.getView().getViewport() == sf::FloatRect({0, 0}, {1, 1}));
            CHECK(window.getView().getTransform() ==
                  Approx(sf::Transform(0.00833333f, 0, -1, 0, -0.00666667f, 1, 0, 0, 1)));
        }
    }

    SECTION("Clear")
    {
        sf::RenderWindow window(graphicsContext,
                                sf::VideoMode(sf::Vector2u{256, 256}, 24),
                                "Window Title",
                                sf::Style::Default,
                                sf::State::Windowed,
                                sf::ContextSettings{});
        REQUIRE(window.getSize() == sf::Vector2u{256, 256});

        auto texture = sf::Texture::create(graphicsContext, sf::Vector2u{256, 256}).value();

        window.clear(sf::Color::Red);
        CHECK(texture.update(window));
        CHECK(texture.copyToImage().getPixel(sf::Vector2u{64, 64}) == sf::Color::Red);

        window.clear(sf::Color::Green);
        CHECK(texture.update(window));
        CHECK(texture.copyToImage().getPixel(sf::Vector2u{128, 128}) == sf::Color::Green);

        window.clear(sf::Color::Blue);
        CHECK(texture.update(window));
        CHECK(texture.copyToImage().getPixel(sf::Vector2u{196, 196}) == sf::Color::Blue);
    }
}
