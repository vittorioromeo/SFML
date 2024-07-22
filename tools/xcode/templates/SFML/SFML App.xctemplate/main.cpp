//
// Disclaimer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// Note that the "Run Script" build phase will copy the required frameworks
// or dylibs to your application bundle so you can execute it on any OS X
// computer.
//
// Your resource files (images, sounds, fonts, ...) are also copied to your
// application bundle. To get the path to these resources, use the helper
// function `resourcePath()` from ResourcePath.hpp
//

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Audio/Music.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/GraphicsContext.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <cstdlib>

// Here is a small helper for you! Have a look.
#include "ResourcePath.hpp"
#include <SFML/Window/EventUtils.hpp>


int main()
{
    // Create the graphics context
    sf::GraphicsContext graphicsContext;

    // Create the main window
    sf::RenderWindow window(graphicsContext, sf::VideoMode({800, 600}), "SFML window");

    // Set the Icon
    const auto icon = sf::Image::loadFromFile(resourcePath() / "icon.png").value();
    window.setIcon(icon);

    // Load a sprite to display
    const auto texture = sf::Texture::loadFromFile(graphicsContext, resourcePath() / "background.jpg").value();
    sf::Sprite sprite(texture.getRect());

    // Create a graphical text to display
    const auto font = sf::Font::openFromFile(graphicsContext, resourcePath() / "tuffy.ttf").value();
    sf::Text   text(font, "Hello SFML", 50);
    text.setFillColor(sf::Color::Black);

    // Load a music to play
    sf::Music music;
    if (!music.openFromFile(resourcePath() / "doodle_pop.ogg"))
    {
        return EXIT_FAILURE;
    }

    // Play the music
    music.play();

    // Start the game loop
    while (true)
    {
        // Process events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        // Clear screen
        window.clear();

        // Draw the sprite
        window.draw(sprite, texture);

        // Draw the string
        window.draw(text);

        // Update the window
        window.display();
    }
}
