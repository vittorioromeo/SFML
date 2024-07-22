// TODO: broken

///////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>

#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/EventUtils.hpp>
#include <SFML/Window/GraphicsContext.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Touch.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <array>
#include <iostream>

#include <cstdlib>

#define GLAD_GL_IMPLEMENTATION
#include <gl.h>

#ifdef SFML_SYSTEM_IOS
#include <SFML/Main.hpp>
#endif

#ifndef GL_SRGB8_ALPHA8
#define GL_SRGB8_ALPHA8 0x8C43
#endif

sf::Path resourcesDir()
{
#ifdef SFML_SYSTEM_IOS
    return "";
#else
    return "resources";
#endif
}

sf::base::Optional<sf::RenderWindow> recreateWindow(sf::GraphicsContext&       graphicsContext,
                                                    const sf::ContextSettings& contextSettings,
                                                    sf::Texture&               texture)
{
    // Create the main window
    sf::base::Optional<sf::RenderWindow> window(sf::base::inPlace,
                                                graphicsContext,
                                                sf::VideoMode({800, 600}),
                                                "SFML graphics with OpenGL",
                                                sf::Style::Default,
                                                sf::State::Windowed,
                                                contextSettings);

    window->setVerticalSyncEnabled(true);
    window->setMinimumSize({400u, 300u});
    window->setMaximumSize({1200u, 900u});

    // Make the window the active window for OpenGL calls
    if (!window->setActive(true))
    {
        std::cerr << "Failed to set window to active" << std::endl;
        return sf::base::nullOpt;
    }

    // Load OpenGL or OpenGL ES entry points using glad
    graphicsContext.loadGLEntryPointsViaGLAD();

    // Enable Z-buffer read and write
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
#ifdef SFML_OPENGL_ES
    glClearDepthf(1.f);
#else
    glClearDepth(1.f);
#endif

    // Disable lighting
    glDisable(GL_LIGHTING);

    // Configure the viewport (the same size as the window)
    glViewport(0, 0, static_cast<GLsizei>(window->getSize().x), static_cast<GLsizei>(window->getSize().y));

    // Setup a perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const GLfloat ratio = static_cast<float>(window->getSize().x) / static_cast<float>(window->getSize().y);
    glFrustum(-ratio, ratio, -1.f, 1.f, 1.f, 500.f);

    // Bind the texture
    glEnable(GL_TEXTURE_2D);
    sf::Texture::bind(graphicsContext, &texture);

    // Define a 3D cube (6 faces made of 2 triangles composed by 3 vertices)
    // clang-format off
    static constexpr std::array<GLfloat, 180> cube =
    {
        // positions    // texture coordinates
        -20, -20, -20,  0, 0,
        -20,  20, -20,  1, 0,
        -20, -20,  20,  0, 1,
        -20, -20,  20,  0, 1,
        -20,  20, -20,  1, 0,
        -20,  20,  20,  1, 1,

         20, -20, -20,  0, 0,
         20,  20, -20,  1, 0,
         20, -20,  20,  0, 1,
         20, -20,  20,  0, 1,
         20,  20, -20,  1, 0,
         20,  20,  20,  1, 1,

        -20, -20, -20,  0, 0,
         20, -20, -20,  1, 0,
        -20, -20,  20,  0, 1,
        -20, -20,  20,  0, 1,
         20, -20, -20,  1, 0,
         20, -20,  20,  1, 1,

        -20,  20, -20,  0, 0,
         20,  20, -20,  1, 0,
        -20,  20,  20,  0, 1,
        -20,  20,  20,  0, 1,
         20,  20, -20,  1, 0,
         20,  20,  20,  1, 1,

        -20, -20, -20,  0, 0,
         20, -20, -20,  1, 0,
        -20,  20, -20,  0, 1,
        -20,  20, -20,  0, 1,
         20, -20, -20,  1, 0,
         20,  20, -20,  1, 1,

        -20, -20,  20,  0, 0,
         20, -20,  20,  1, 0,
        -20,  20,  20,  0, 1,
        -20,  20,  20,  0, 1,
         20, -20,  20,  1, 0,
         20,  20,  20,  1, 1
    };
    // clang-format on

    // Enable position and texture coordinates vertex components
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), cube.data());
    glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), cube.data() + 3);

    // Disable normal and color vertex components
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // Make the window no longer the active window for OpenGL calls
    if (!window->setActive(false))
    {
        std::cerr << "Failed to set window to inactive" << std::endl;
        return sf::base::nullOpt;
    }

    return window;
}

////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    bool sRgb = false;

    // Create the graphics context
    sf::GraphicsContext graphicsContext;

    while (true)
    {
        // Request a 24-bits depth buffer when creating the window
        sf::ContextSettings contextSettings;
        contextSettings.depthBits   = 24;
        contextSettings.sRgbCapable = sRgb;

        // Create a sprite for the background
        const auto backgroundTexture = sf::Texture::loadFromFile(graphicsContext, resourcesDir() / "background.jpg", sRgb)
                                           .value();

        const sf::Sprite background(backgroundTexture.getRect());

        // Create some text to draw on top of our OpenGL object
        const auto font = sf::Font::openFromFile(graphicsContext, resourcesDir() / "tuffy.ttf").value();

        sf::Text text(font, "SFML / OpenGL demo");
        sf::Text sRgbInstructions(font, "Press space to toggle sRGB conversion (off)");
        sf::Text mipmapInstructions(font, "Press return to toggle mipmapping (on)");
        text.setFillColor(sf::Color(255, 255, 255, 170));
        sRgbInstructions.setFillColor(sf::Color(255, 255, 255, 170));
        mipmapInstructions.setFillColor(sf::Color(255, 255, 255, 170));
        text.setPosition({280.f, 450.f});
        sRgbInstructions.setPosition({175.f, 500.f});
        mipmapInstructions.setPosition({200.f, 550.f});

        // Load a texture to apply to our 3D cube
        auto texture = sf::Texture::loadFromFile(graphicsContext, resourcesDir() / "logo.png").value();

        // Attempt to generate a mipmap for our cube texture
        // We don't check the return value here since
        // mipmapping is purely optional in this example
        (void)texture.generateMipmap();

        // Create the main window
        sf::base::Optional<sf::RenderWindow> window = recreateWindow(graphicsContext, contextSettings, texture);
        if (!window.hasValue())
            return EXIT_FAILURE;

        // Create a clock for measuring the time elapsed
        const sf::Clock clock;

        // Flag to track whether mipmapping is currently enabled
        bool mipmapEnabled = true;

        // Start game loop
        while (true)
        {
            // Process events
            while (const sf::base::Optional event = window->pollEvent())
            {
                if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                    return EXIT_SUCCESS;

                // Return key: toggle mipmapping
                if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                    keyPressed && keyPressed->code == sf::Keyboard::Key::Enter)
                {
                    if (mipmapEnabled)
                    {
                        // We simply reload the texture to disable mipmapping
                        texture = sf::Texture::loadFromFile(graphicsContext, resourcesDir() / "logo.png").value();

                        // Rebind the texture
                        sf::Texture::bind(graphicsContext, &texture);

                        mipmapEnabled = false;
                    }
                    else if (texture.generateMipmap())
                    {
                        mipmapEnabled = true;
                    }

                    mipmapInstructions.setString(mipmapEnabled ? "Press return to toggle mipmapping (on)"
                                                               : "Press return to toggle mipmapping (off)");
                }

                // Space key: toggle sRGB conversion
                if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
                    keyPressed && keyPressed->code == sf::Keyboard::Key::Space)
                {
                    sRgb = !sRgb;

                    sRgbInstructions.setString(sRgb ? "Press space to toggle sRGB conversion (on)"
                                                    : "Press space to toggle sRGB conversion (off)");

                    window.reset();
                    break;
                }

                // Adjust the viewport when the window is resized
                if (const auto* resized = event->getIf<sf::Event::Resized>())
                {
                    const sf::Vector2u textureSize = backgroundTexture.getSize();

                    // Make the window the active window for OpenGL calls
                    if (!window->setActive(true))
                    {
                        std::cerr << "Failed to set window to active" << std::endl;
                        return EXIT_FAILURE;
                    }

                    const auto [width, height] = resized->size;
                    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
                    glMatrixMode(GL_PROJECTION);
                    glLoadIdentity();
                    const GLfloat newRatio = static_cast<float>(width) / static_cast<float>(height);
                    glFrustum(-newRatio, newRatio, -1.f, 1.f, 1.f, 500.f);

                    // Make the window no longer the active window for OpenGL calls
                    if (!window->setActive(false))
                    {
                        std::cerr << "Failed to set window to inactive" << std::endl;
                        return EXIT_FAILURE;
                    }

                    sf::View view;
                    view.setSize(textureSize.to<sf::Vector2f>());
                    view.setCenter(textureSize.to<sf::Vector2f>() / 2.f);
                    window->setView(view);
                }
            }

            if (!window.hasValue()) // re-create the window
            {
                window = recreateWindow(graphicsContext, contextSettings, texture);

                if (!window.hasValue())
                    return EXIT_FAILURE;
            }

            // Draw the background
            window->pushGLStates();
            window->draw(background, backgroundTexture);
            window->popGLStates();

            // Clear the depth buffer
            glClear(GL_DEPTH_BUFFER_BIT);

            // We get the position of the mouse cursor (or touch), so that we can move the box accordingly
            sf::Vector2i pos;

#ifdef SFML_SYSTEM_IOS
            pos = sf::Touch::getPosition(0);
#else
            pos = sf::Mouse::getPosition(*window);
#endif

            const float x = static_cast<float>(pos.x) * 200.f / static_cast<float>(window->getSize().x) - 100.f;
            const float y = -static_cast<float>(pos.y) * 200.f / static_cast<float>(window->getSize().y) + 100.f;

            // Apply some transformations
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(x, y, -100.f);
            glRotatef(clock.getElapsedTime().asSeconds() * 50.f, 1.f, 0.f, 0.f);
            glRotatef(clock.getElapsedTime().asSeconds() * 30.f, 0.f, 1.f, 0.f);
            glRotatef(clock.getElapsedTime().asSeconds() * 90.f, 0.f, 0.f, 1.f);

            // Draw the cube
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // Make the window no longer the active window for OpenGL calls
            if (!window->setActive(false))
            {
                std::cerr << "Failed to set window to inactive" << std::endl;
                return EXIT_FAILURE;
            }

            // Draw some text on top of our OpenGL object
            window->pushGLStates();
            window->draw(text);
            window->draw(sRgbInstructions);
            window->draw(mipmapInstructions);
            window->popGLStates();

            // Finally, display the rendered frame on screen
            window->display();
        }
    }

    return EXIT_SUCCESS;
}
