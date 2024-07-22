#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/EventUtils.hpp>
#if 1

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/GraphicsContext.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/Path.hpp>
#include <SFML/System/String.hpp>

#include <array>
#include <sstream>


////////////////////////////////////////////////////////////

// change this to 1 to trigger the bug
#define TRIGGER_THE_BUG 1

int main()
{
    const float screenWidth  = 800.0f;
    const float screenHeight = 600.0f;

    const sf::Vector2u screenSize{static_cast<unsigned int>(screenWidth), static_cast<unsigned int>(screenHeight)};

    sf::GraphicsContext graphicsContext;

    sf::RenderWindow window(graphicsContext,
                            sf::VideoMode(screenSize),
                            "Window",
                            sf::Style::Default,
                            sf::State::Windowed,
                            sf::ContextSettings(0, 0, 4));

    window.setVerticalSyncEnabled(true);

    auto image   = sf::Image::create(screenSize, sf::Color::White).value();
    auto texture = sf::Texture::loadFromImage(graphicsContext, image).value();

    auto baseRenderTexture = sf::RenderTexture::create(graphicsContext, screenSize, sf::ContextSettings{0, 0, 4 /* AA level*/})
                                 .value();

    sf::RenderTexture renderTextures
        [2]{sf::RenderTexture::create(graphicsContext, screenSize, sf::ContextSettings{0, 0, 4 /* AA level*/}).value(),
            sf::RenderTexture::create(graphicsContext, screenSize, sf::ContextSettings{0, 0, 4 /* AA level*/}).value()};

    std::vector<sf::Vertex> vertexArrays[2];

    while (true)
    {
        while (auto event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        window.clear();

        vertexArrays[0].clear();
        vertexArrays[1].clear();

        float xCenter = screenWidth / 2;

        vertexArrays[0].emplace_back(sf::Vector2f{0, 0}, sf::Color::White, sf::Vector2f{0, 0});
        vertexArrays[0].emplace_back(sf::Vector2f{xCenter, 0}, sf::Color::White, sf::Vector2f{xCenter, 0});
        vertexArrays[0].emplace_back(sf::Vector2f{0, screenHeight}, sf::Color::White, sf::Vector2f{0, screenHeight});

        vertexArrays[0].emplace_back(sf::Vector2f{0, screenHeight}, sf::Color::White, sf::Vector2f{0, screenHeight});
        vertexArrays[0].emplace_back(sf::Vector2f{xCenter, 0}, sf::Color::White, sf::Vector2f{xCenter, 0});
        vertexArrays[0].emplace_back(sf::Vector2f{xCenter, screenHeight}, sf::Color::White, sf::Vector2f{xCenter, screenHeight});

        // right half of screen
        vertexArrays[1].emplace_back(sf::Vector2f{xCenter, 0}, sf::Color::White, sf::Vector2f{xCenter, 0});
        vertexArrays[1].emplace_back(sf::Vector2f{screenWidth, 0}, sf::Color::White, sf::Vector2f{screenWidth, 0});
        vertexArrays[1].emplace_back(sf::Vector2f{xCenter, screenHeight}, sf::Color::White, sf::Vector2f{xCenter, screenHeight});

        vertexArrays[1].emplace_back(sf::Vector2f{xCenter, screenHeight}, sf::Color::White, sf::Vector2f{xCenter, screenHeight});
        vertexArrays[1].emplace_back(sf::Vector2f{screenWidth, 0}, sf::Color::White, sf::Vector2f{screenWidth, 0});
        vertexArrays[1].emplace_back(sf::Vector2f{screenWidth, screenHeight},
                                     sf::Color::White,
                                     sf::Vector2f{screenWidth, screenHeight});

        renderTextures[0].clear();
        renderTextures[1].clear();

        sf::Sprite sprite(texture.getRect());
        renderTextures[0].draw(sprite, texture);

        sprite.setColor(sf::Color::Green);
        renderTextures[1].draw(sprite, texture);

        baseRenderTexture.clear();


        renderTextures[0].display();
        baseRenderTexture.draw(vertexArrays[0],
                               sf::PrimitiveType::Triangles,
                               sf::RenderStates{&renderTextures[0].getTexture()});

        renderTextures[1].display();
        baseRenderTexture.draw(vertexArrays[1],
                               sf::PrimitiveType::Triangles,
                               sf::RenderStates{&renderTextures[1].getTexture()});

        baseRenderTexture.display();

        window.draw(sf::Sprite(baseRenderTexture.getTexture().getRect()), baseRenderTexture.getTexture());

        window.display();
    }

    return 0;
}

#elif defined(BARABARAR)

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Path.hpp>
#include <SFML/System/String.hpp>

#include <sstream>

int main()
{
    sf::GraphicsContext graphicsContext;

    sf::RenderWindow window(graphicsContext, sf::VideoMode({800u, 600u}), "Test", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(false);

    const auto font = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf").value();

    sf::Text text(font, "Test", 20);

    sf::RenderTexture renderTexture[10]{sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value()};

    ;

    sf::Clock          clock;
    std::ostringstream sstr;

    while (true)
    {
        while (auto event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        for (int i = 0; i < 20; ++i)
        {
            for (auto& j : renderTexture)
            {
                j.clear(sf::Color(0, 0, 0));

                for (auto& u : renderTexture)
                    u.draw(text);

                j.display();
            }

            window.clear(sf::Color(0, 0, 0));

            for (const auto& j : renderTexture)
            {
                sf::Sprite sprite(j.getTexture().getRect());

                for (int k = 0; k < 10; ++k)
                    window.draw(sprite, j.getTexture());
            }
        }

        window.display();

        sstr.str("");
        sstr << "Test -- Frame: " << clock.restart().asSeconds() << " sec";

        window.setTitle(sstr.str());
    }

    return EXIT_SUCCESS;
}

#else

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Path.hpp>
#include <SFML/System/String.hpp>

#include <cstdlib>

int main()
{
    sf::GraphicsContext graphicsContext;

    const auto       font         = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf").value();
    const sf::String textContents = "abcdefghilmnopqrstuvz\nabcdefghilmnopqrstuvz\nabcdefghilmnopqrstuvz\n";

    auto text          = sf::Text(font, textContents);
    auto renderTexture = sf::RenderTexture::create(graphicsContext, {1680, 1050}).value();

    renderTexture.clear();

    for (std::size_t i = 0; i < 100'000; ++i)
    {
        text.setOutlineThickness(static_cast<float>(5 + (i % 2)));
        renderTexture.draw(text);
    }

    renderTexture.display();
}

#endif
