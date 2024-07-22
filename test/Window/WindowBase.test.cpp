#include <SFML/Window/WindowBase.hpp>

// Other 1st party headers
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/String.hpp>
#include <SFML/System/TimeChronoUtil.hpp>

#include <Doctest.hpp>

#include <StringifyOptionalUtil.hpp>
#include <SystemUtil.hpp>
#include <WindowUtil.hpp>

#include <chrono>
#include <type_traits>

TEST_CASE("[Window] sf::WindowBase" * doctest::skip(skipDisplayTests))
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!std::has_virtual_destructor_v<sf::WindowBase>);
        STATIC_CHECK(!std::is_copy_constructible_v<sf::WindowBase>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::WindowBase>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::WindowBase>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::WindowBase>);
    }

    SECTION("Construction")
    {
        SECTION("Mode and title constructor")
        {
            const sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");
            CHECK(windowBase.getSize() == sf::Vector2u{360, 240});
            CHECK(windowBase.getNativeHandle() != sf::WindowHandle());
        }

        SECTION("Mode, title, and style constructor")
        {
            const sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests", sf::Style::Resize);
            CHECK(windowBase.getSize() == sf::Vector2u{360, 240});
            CHECK(windowBase.getNativeHandle() != sf::WindowHandle());
        }

        SECTION("Mode, title, style, and state constructor")
        {
            const sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests", sf::Style::Resize, sf::State::Windowed);
            CHECK(windowBase.getSize() == sf::Vector2u{360, 240});
            CHECK(windowBase.getNativeHandle() != sf::WindowHandle());
        }

        SECTION("Mode, title, and state constructor")
        {
            const sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests", sf::State::Windowed);
            CHECK(windowBase.getSize() == sf::Vector2u{360, 240});
            CHECK(windowBase.getNativeHandle() != sf::WindowHandle());
        }
    }

    SECTION("waitEvent()")
    {
        SECTION("Initialized window")
        {
            sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");

            constexpr auto timeout = sf::milliseconds(50);

            const auto startTime = std::chrono::steady_clock::now();
            const auto event     = windowBase.waitEvent(timeout);
            const auto elapsed   = std::chrono::steady_clock::now() - startTime;

            REQUIRE(elapsed < sf::TimeChronoUtil::toDuration(timeout + sf::milliseconds(50)));

            if (elapsed <= sf::TimeChronoUtil::toDuration(timeout))
                CHECK(event);
            else
                CHECK(!event);
        }
    }

    SECTION("Set/get position")
    {
        sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");
        windowBase.setPosition({12, 34});
        CHECK(windowBase.getPosition() == sf::Vector2i());
    }

    SECTION("Set/get size")
    {
        SECTION("Uninitialized window")
        {
            sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");
            windowBase.setSize({128, 256});
            CHECK(windowBase.getSize() == sf::Vector2u());
        }

        SECTION("Initialized window")
        {
            sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");
            windowBase.setSize({128, 256});
            CHECK(windowBase.getSize() == sf::Vector2u{128, 256});
        }

        SECTION("Minimum size")
        {
            sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");
            windowBase.setMinimumSize({128u, 256u});
            windowBase.setSize({100, 100});
            CHECK(windowBase.getSize() == sf::Vector2u{128, 256});
        }

        SECTION("Maximum size")
        {
            sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");
            windowBase.setMaximumSize({128u, 256u});
            windowBase.setSize({400, 400});
            CHECK(windowBase.getSize() == sf::Vector2u{128, 256});
        }
    }

    SECTION("setMinimumSize()")
    {
        sf::WindowBase windowBase(sf::VideoMode({100, 100}), "WindowBase Tests", sf::Style::Default ^ sf::Style::Resize);
        windowBase.setMinimumSize({200u, 300u});
        CHECK(windowBase.getSize() == sf::Vector2u{200, 300});
        windowBase.setMaximumSize({200u, 300u});
    }

    SECTION("setMinimumSize()")
    {
        sf::WindowBase windowBase(sf::VideoMode({400, 400}), "WindowBase Tests", sf::Style::Default ^ sf::Style::Resize);
        windowBase.setMaximumSize({200u, 300u});
        CHECK(windowBase.getSize() == sf::Vector2u{200, 300});
        windowBase.setMinimumSize({200u, 300u});
    }

    // Test for compilation but do not run. This code sometimes hangs indefinitely
    // when running on the BuildBot CI pipeline. Because it contains no
    // assertions we have nothing to gain by running it anyways
    (void)[]
    {
        sf::WindowBase windowBase(sf::VideoMode({360, 240}), "WindowBase Tests");

        // Should compile if user provides only a specific handler
        windowBase.pollAndHandleEvents([](const sf::Event::Closed&) {});

        // Should compile if user provides only a catch-all
        windowBase.pollAndHandleEvents([](const auto&) {});

        // Should compile if user provides both a specific handler and a catch-all
        windowBase.pollAndHandleEvents([](const sf::Event::Closed&) {}, [](const auto&) {});
    };
}
