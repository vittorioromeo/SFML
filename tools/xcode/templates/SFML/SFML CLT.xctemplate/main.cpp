//
// Disclaimer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// In order to load the resources like background.png, you have to set up
// your target scheme:
//
// - Select "Edit Scheme…" in the "Product" menu;
// - Check the box "use custom working directory";
// - Fill the text field with the folder path containing your resources;
//        (e.g. your project folder)
// - Click OK.
//

#include <SFML/Window/EventUtils.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/Audio/Music.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <cstdlib>

int main()
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML window");

    // Set the Icon
    const auto icon = sf::Image::loadFromFile("icon.png").value();
    window.setIcon(icon);

    // Load a sprite to display
    const auto texture = sf::Texture::loadFromFile("background.jpg").value();
    sf::Sprite sprite(texture.getRect());

    // Create a graphical text to display
    const auto font = sf::Font::openFromFile("tuffy.ttf").value();
    sf::Text   text(font, "Hello SFML", 50);
    text.setFillColor(sf::Color::Black);

    // Load a music to play
    sf::Music music;
    if (!music.openFromFile("doodle_pop.ogg"))
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
