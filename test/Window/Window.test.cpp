#include <SFML/Window/GraphicsContext.hpp>
#include <SFML/Window/Window.hpp>

// Other 1st party headers
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/String.hpp>

#include <Doctest.hpp>

#include <SystemUtil.hpp>
#include <WindowUtil.hpp>

#include <type_traits>

TEST_CASE("[Window] sf::Window" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::has_virtual_destructor_v<sf::Window>);
        STATIC_CHECK(!std::is_copy_constructible_v<sf::Window>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::Window>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::Window>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::Window>);
    }

    SECTION("Construction")
    {
        SECTION("Mode and title constructor")
        {
            const sf::Window window(graphicsContext, sf::VideoMode({360, 240}), "Window Tests");
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, and style constructor")
        {
            const sf::Window window(graphicsContext, sf::VideoMode({360, 240}), "Window Tests", sf::Style::Resize);
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, and state constructor")
        {
            const sf::Window
                window(graphicsContext, sf::VideoMode({360, 240}), "Window Tests", sf::Style::Resize, sf::State::Windowed);
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, style, state, and context settings constructor")
        {
            const sf::Window window(graphicsContext,
                                    sf::VideoMode({360, 240}),
                                    "Window Tests",
                                    sf::Style::Resize,
                                    sf::State::Windowed,
                                    sf::ContextSettings{/* depthBits*/ 1, /* stencilBits */ 1, /* antialiasingLevel */ 1});
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().depthBits >= 1);
            CHECK(window.getSettings().stencilBits >= 1);
            CHECK(window.getSettings().antialiasingLevel >= 1);
        }

        SECTION("Mode, title, and state")
        {
            const sf::Window window(graphicsContext, sf::VideoMode({360, 240}), "Window Tests", sf::State::Windowed);
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().attributeFlags == sf::ContextSettings{}.attributeFlags);
        }

        SECTION("Mode, title, state, and context settings constructor")
        {
            const sf::Window window(graphicsContext,
                                    sf::VideoMode({360, 240}),
                                    "Window Tests",
                                    sf::State::Windowed,
                                    sf::ContextSettings{/* depthBits*/ 1, /* stencilBits */ 1, /* antialiasingLevel */ 1});
            CHECK(window.getSize() == sf::Vector2u{360, 240});
            CHECK(window.getNativeHandle() != sf::WindowHandle());
            CHECK(window.getSettings().depthBits >= 1);
            CHECK(window.getSettings().stencilBits >= 1);
            CHECK(window.getSettings().antialiasingLevel >= 1);
        }
    }
}
