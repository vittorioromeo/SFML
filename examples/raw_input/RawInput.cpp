////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/EventUtils.hpp>
#include <SFML/Window/GraphicsContext.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/Path.hpp>
#include <SFML/System/String.hpp>

#include <string>
#include <vector>

#include <cstdlib>


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the graphics context
    sf::GraphicsContext graphicsContext;

    // Create the main window
    sf::RenderWindow window(graphicsContext,
                            sf::VideoMode({800u, 600u}),
                            "SFML Raw Mouse Input",
                            sf::Style::Titlebar | sf::Style::Close);

    window.setVerticalSyncEnabled(true);

    // Open the application font and pass it to the Effect class
    const auto font = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf").value();

    // Create the mouse position text
    sf::Text mousePosition(font, "", 20);
    mousePosition.setPosition({400.f, 300.f});
    mousePosition.setFillColor(sf::Color::White);

    // Create the mouse raw movement text
    sf::Text mouseRawMovement(font, "", 20);
    mouseRawMovement.setFillColor(sf::Color::White);

    std::vector<std::string> log;

    while (true)
    {
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;

            static const auto vec2ToString = [](const sf::Vector2i vec2)
            { return '(' + std::to_string(vec2.x) + ", " + std::to_string(vec2.y) + ')'; };

            if (const auto* const mouseMoved = event->getIf<sf::Event::MouseMoved>())
                mousePosition.setString("Mouse Position: " + vec2ToString(mouseMoved->position));

            if (const auto* const mouseMovedRaw = event->getIf<sf::Event::MouseMovedRaw>())
            {
                log.emplace_back("Mouse Movement: " + vec2ToString(mouseMovedRaw->delta));

                if (log.size() > 24u)
                    log.erase(log.begin());
            }
        }

        window.clear();
        window.draw(mousePosition);

        for (std::size_t i = 0; i < log.size(); ++i)
        {
            mouseRawMovement.setPosition({50.f, static_cast<float>(i * 20) + 50.f});
            mouseRawMovement.setString(log[i]);
            window.draw(mouseRawMovement);
        }

        window.display();
    }
}
