////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/EventUtils.hpp>
#include <SFML/Window/GraphicsContext.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/Clock.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Time.hpp>

#include <SFML/Base/Optional.hpp>

#include <array>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <cmath>
#include <cstdint>
#include <cstdlib>


namespace
{
std::random_device rd;
std::mt19937       rng(rd());


////////////////////////////////////////////////////////////
// Base class for effects
////////////////////////////////////////////////////////////
struct Effect
{
    virtual ~Effect() = default;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
    virtual void update(float time, float x, float y)                          = 0;
};


////////////////////////////////////////////////////////////
// "Pixelate" fragment shader
////////////////////////////////////////////////////////////
class Pixelate : public Effect
{
public:
    explicit Pixelate(sf::Texture&& texture, sf::Shader&& shader) :
    m_texture(std::move(texture)),
    m_shader(std::move(shader)),
    m_ulTexture(m_shader.getUniformLocation("sf_u_texture").value()),
    m_ulPixelThreshold(m_shader.getUniformLocation("pixel_threshold").value())
    {
        m_shader.setUniform(m_ulTexture, sf::Shader::CurrentTexture);
    }

    void update(float /* time */, float x, float y) override
    {
        m_shader.setUniform(m_ulPixelThreshold, (x + y) / 30);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.shader = &m_shader;
        target.draw(sf::Sprite{m_texture.getRect()}, m_texture, states);
    }

private:
    sf::Texture                 m_texture;
    sf::Shader                  m_shader;
    sf::Shader::UniformLocation m_ulTexture;
    sf::Shader::UniformLocation m_ulPixelThreshold;
};


////////////////////////////////////////////////////////////
// "Wave" vertex shader + "blur" fragment shader
////////////////////////////////////////////////////////////
class WaveBlur : public Effect
{
public:
    void update(float time, float x, float y) override
    {
        m_shader.setUniform(m_ulWavePhase, time);
        m_shader.setUniform(m_ulWaveAmplitude, sf::Vector2f(x * 40, y * 40));
        m_shader.setUniform(m_ulBlurRadius, (x + y) * 0.008f);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.shader = &m_shader;
        target.draw(m_text, states);
    }

    explicit WaveBlur(const sf::Font& font, sf::Shader&& shader) :
    m_text(font,
           "Praesent suscipit augue in velit pulvinar hendrerit varius purus aliquam.\n"
           "Mauris mi odio, bibendum quis fringilla a, laoreet vel orci. Proin vitae vulputate tortor.\n"
           "Praesent cursus ultrices justo, ut feugiat ante vehicula quis.\n"
           "Donec fringilla scelerisque mauris et viverra.\n"
           "Maecenas adipiscing ornare scelerisque. Nullam at libero elit.\n"
           "Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas.\n"
           "Nullam leo urna, tincidunt id semper eget, ultricies sed mi.\n"
           "Morbi mauris massa, commodo id dignissim vel, lobortis et elit.\n"
           "Fusce vel libero sed neque scelerisque venenatis.\n"
           "Integer mattis tincidunt quam vitae iaculis.\n"
           "Vivamus fringilla sem non velit venenatis fermentum.\n"
           "Vivamus varius tincidunt nisi id vehicula.\n"
           "Integer ullamcorper, enim vitae euismod rutrum, massa nisl semper ipsum,\n"
           "vestibulum sodales sem ante in massa.\n"
           "Vestibulum in augue non felis convallis viverra.\n"
           "Mauris ultricies dolor sed massa convallis sed aliquet augue fringilla.\n"
           "Duis erat eros, porta in accumsan in, blandit quis sem.\n"
           "In hac habitasse platea dictumst. Etiam fringilla est id odio dapibus sit amet semper dui laoreet.\n",
           22),
    m_shader(std::move(shader)),
    m_ulWavePhase(m_shader.getUniformLocation("wave_phase").value()),
    m_ulWaveAmplitude(m_shader.getUniformLocation("wave_amplitude").value()),
    m_ulBlurRadius(m_shader.getUniformLocation("blur_radius").value())
    {
        m_text.setPosition({30.f, 20.f});
    }

private:
    sf::Text                    m_text;
    sf::Shader                  m_shader;
    sf::Shader::UniformLocation m_ulWavePhase;
    sf::Shader::UniformLocation m_ulWaveAmplitude;
    sf::Shader::UniformLocation m_ulBlurRadius;
};


////////////////////////////////////////////////////////////
// "Storm" vertex shader + "blink" fragment shader
////////////////////////////////////////////////////////////
class StormBlink : public Effect
{
public:
    void update(float time, float x, float y) override
    {
        const float radius = 200 + std::cos(time) * 150;

        m_shader.setUniform(m_ulStormPosition, sf::Vector2f(x * 800, y * 600));
        m_shader.setUniform(m_ulStormInnerRadius, radius / 3);
        m_shader.setUniform(m_ulStormTotalRadius, radius);
        m_shader.setUniform(m_ulBlinkAlpha, 0.5f + std::cos(time * 3) * 0.25f);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        states.shader = &m_shader;
        target.draw(m_points, sf::PrimitiveType::Points, states);
    }

    explicit StormBlink(sf::Shader&& shader) :
    m_shader(std::move(shader)),
    m_ulStormPosition(m_shader.getUniformLocation("storm_position").value()),
    m_ulStormInnerRadius(m_shader.getUniformLocation("storm_inner_radius").value()),
    m_ulStormTotalRadius(m_shader.getUniformLocation("storm_total_radius").value()),
    m_ulBlinkAlpha(m_shader.getUniformLocation("blink_alpha").value())
    {
        std::uniform_real_distribution<float>        xDistribution(0, 800);
        std::uniform_real_distribution<float>        yDistribution(0, 600);
        std::uniform_int_distribution<std::uint16_t> colorDistribution(0, 255);

        // Create the points
        for (int i = 0; i < 40000; ++i)
        {
            const auto x = xDistribution(rng);
            const auto y = yDistribution(rng);

            const auto r = static_cast<std::uint8_t>(colorDistribution(rng));
            const auto g = static_cast<std::uint8_t>(colorDistribution(rng));
            const auto b = static_cast<std::uint8_t>(colorDistribution(rng));

            m_points.push_back({{x, y}, {r, g, b}});
        }
    }

private:
    std::vector<sf::Vertex>     m_points;
    sf::Shader                  m_shader;
    sf::Shader::UniformLocation m_ulStormPosition;
    sf::Shader::UniformLocation m_ulStormInnerRadius;
    sf::Shader::UniformLocation m_ulStormTotalRadius;
    sf::Shader::UniformLocation m_ulBlinkAlpha;
};


////////////////////////////////////////////////////////////
// "Edge" post-effect fragment shader
////////////////////////////////////////////////////////////
class Edge : public Effect
{
public:
    void update(float time, float x, float y) override
    {
        m_shader.setUniform(m_ulEdgeThreshold, sf::base::clamp(1.f - (x + y) / 2.f, 0.f, 1.f));

        // Render the updated scene to the off-screen surface
        m_surface.clear(sf::Color::White);

        sf::Sprite backgroundSprite{m_backgroundTexture.getRect()};
        backgroundSprite.setPosition({135.f, 100.f});
        m_surface.draw(backgroundSprite, m_backgroundTexture);

        // Update the position of the moving entities
        constexpr int numEntities = 6;

        for (int i = 0; i < 6; ++i)
        {
            sf::Sprite entity{{{96 * i, 0}, {96, 96}}};

            entity.setPosition(
                {std::cos(0.25f * (time * static_cast<float>(i) + static_cast<float>(numEntities - i))) * 300 + 350,
                 std::sin(0.25f * (time * static_cast<float>(numEntities - i) + static_cast<float>(i))) * 200 + 250});

            m_surface.draw(entity, m_entityTexture);
        }

        m_surface.display();
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        const sf::Texture& texture = m_surface.getTexture();

        states.shader = &m_shader;
        target.draw(sf::Sprite{texture.getRect()}, texture, states);
    }

    explicit Edge(sf::RenderTexture&& surface, sf::Texture&& backgroundTexture, sf::Texture&& entityTexture, sf::Shader&& shader) :
    m_surface(std::move(surface)),
    m_backgroundTexture(std::move(backgroundTexture)),
    m_entityTexture(std::move(entityTexture)),
    m_shader(std::move(shader)),
    m_ulEdgeThreshold(m_shader.getUniformLocation("edge_threshold").value())
    {
    }

private:
    sf::RenderTexture           m_surface;
    sf::Texture                 m_backgroundTexture;
    sf::Texture                 m_entityTexture;
    sf::Shader                  m_shader;
    sf::Shader::UniformLocation m_ulEdgeThreshold;
};


////////////////////////////////////////////////////////////
// "Geometry" geometry shader example
////////////////////////////////////////////////////////////
class Geometry : public Effect
{
public:
    void update(float /* time */, float x, float y) override
    {
        // Reset our transformation matrix
        m_transform = sf::Transform::Identity;

        // Move to the center of the window
        m_transform.translate({400.f, 300.f});

        // Rotate everything based on cursor position
        m_transform.rotate(sf::degrees(x * 360.f));

        // Adjust billboard size to scale between 25 and 75
        const float size = 25 + std::abs(y) * 50;

        // Update the shader parameter
        m_shader.setUniform(m_ulSize, sf::Vector2f{size, size});
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        // Prepare the render state
        states.shader    = &m_shader;
        states.texture   = &m_logoTexture;
        states.transform = m_transform;

        // Draw the point cloud
        target.draw(m_pointCloud, sf::PrimitiveType::Points, states);
    }

    explicit Geometry(sf::Texture&& logoTexture, sf::Shader&& shader) :
    m_logoTexture(std::move(logoTexture)),
    m_shader(std::move(shader)),
    m_ulSize(m_shader.getUniformLocation("size").value()),
    m_pointCloud(10000)
    {
        // Move the points in the point cloud to random positions
        for (std::size_t i = 0; i < 10000; ++i)
        {
            // Spread the coordinates from -480 to +480 so they'll always fill the viewport at 800x600
            std::uniform_real_distribution<float> positionDistribution(-480, 480);
            m_pointCloud[i].position = {positionDistribution(rng), positionDistribution(rng)};
        }
    }

private:
    sf::Texture                 m_logoTexture;
    sf::Transform               m_transform;
    sf::Shader                  m_shader;
    sf::Shader::UniformLocation m_ulSize;
    std::vector<sf::Vertex>     m_pointCloud;
};


////////////////////////////////////////////////////////////
// Effect loading factory functions
////////////////////////////////////////////////////////////
sf::base::Optional<Pixelate> tryLoadPixelate(sf::GraphicsContext& graphicsContext)
{
    auto texture = sf::Texture::loadFromFile(graphicsContext, "resources/background.jpg");
    if (!texture.hasValue())
        return sf::base::nullOpt;

    auto shader = sf::Shader::loadFromFile(graphicsContext, "resources/billboard.vert", "resources/pixelate.frag");
    if (!shader.hasValue())
        return sf::base::nullOpt;

    return sf::base::makeOptional<Pixelate>(std::move(*texture), std::move(*shader));
}

sf::base::Optional<WaveBlur> tryLoadWaveBlur(sf::GraphicsContext& graphicsContext, const sf::Font& font)
{
    auto shader = sf::Shader::loadFromFile(graphicsContext, "resources/wave.vert", "resources/blur.frag");
    if (!shader.hasValue())
        return sf::base::nullOpt;

    return sf::base::makeOptional<WaveBlur>(font, std::move(*shader));
}

sf::base::Optional<StormBlink> tryLoadStormBlink(sf::GraphicsContext& graphicsContext)
{
    auto shader = sf::Shader::loadFromFile(graphicsContext, "resources/storm.vert", "resources/blink.frag");
    if (!shader.hasValue())
        return sf::base::nullOpt;

    return sf::base::makeOptional<StormBlink>(std::move(*shader));
}

sf::base::Optional<Edge> tryLoadEdge(sf::GraphicsContext& graphicsContext)
{
    // Create the off-screen surface
    auto surface = sf::RenderTexture::create(graphicsContext, {800, 600});
    if (!surface.hasValue())
        return sf::base::nullOpt;

    surface->setSmooth(true);

    // Load the background texture
    auto backgroundTexture = sf::Texture::loadFromFile(graphicsContext, "resources/sfml.png");
    if (!backgroundTexture.hasValue())
        return sf::base::nullOpt;

    backgroundTexture->setSmooth(true);

    // Load the entity texture
    auto entityTexture = sf::Texture::loadFromFile(graphicsContext, "resources/devices.png");
    if (!entityTexture.hasValue())
        return sf::base::nullOpt;

    entityTexture->setSmooth(true);

    // Load the shader
    auto shader = sf::Shader::loadFromFile(graphicsContext, "resources/billboard.vert", "resources/edge.frag");
    if (!shader.hasValue())
        return sf::base::nullOpt;

    shader->setUniform(shader->getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);

    return sf::base::makeOptional<Edge>(std::move(*surface),
                                        std::move(*backgroundTexture),
                                        std::move(*entityTexture),
                                        std::move(*shader));
}

sf::base::Optional<Geometry> tryLoadGeometry(sf::GraphicsContext& graphicsContext)
{
    // Check if geometry shaders are supported
    if (!sf::Shader::isGeometryAvailable(graphicsContext))
        return sf::base::nullOpt;

    // Load the logo texture
    auto logoTexture = sf::Texture::loadFromFile(graphicsContext, "resources/logo.png");
    if (!logoTexture.hasValue())
        return sf::base::nullOpt;

    logoTexture->setSmooth(true);

    // Load the shader
    auto shader = sf::Shader::loadFromFile(graphicsContext,
                                           "resources/billboard.vert",
                                           "resources/billboard.geom",
                                           "resources/billboard.frag");
    if (!shader.hasValue())
        return sf::base::nullOpt;

    shader->setUniform(shader->getUniformLocation("sf_u_texture").value(), sf::Shader::CurrentTexture);

    // Set the render resolution (used for proper scaling)
    shader->setUniform(shader->getUniformLocation("resolution").value(), sf::Vector2f{800, 600});

    return sf::base::makeOptional<Geometry>(std::move(*logoTexture), std::move(*shader));
}

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the graphics context
    sf::GraphicsContext graphicsContext;

    // Open the application font
    const auto font = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf").value();

    // Create the effects
    sf::base::Optional pixelateEffect   = tryLoadPixelate(graphicsContext);
    sf::base::Optional waveBlurEffect   = tryLoadWaveBlur(graphicsContext, font);
    sf::base::Optional stormBlinkEffect = tryLoadStormBlink(graphicsContext);
    sf::base::Optional edgeEffect       = tryLoadEdge(graphicsContext);
    sf::base::Optional geometryEffect   = tryLoadGeometry(graphicsContext);

    const std::array<Effect*, 5> effects{pixelateEffect.asPtr(),
                                         waveBlurEffect.asPtr(),
                                         stormBlinkEffect.asPtr(),
                                         edgeEffect.asPtr(),
                                         geometryEffect.asPtr()};

    const std::array<std::string, 5>
        effectNames{"Pixelate", "Wave + Blur", "Storm + Blink", "Edge Post-effect", "Geometry Shader Billboards"};

    // Index of currently selected effect
    std::size_t current = 0;

    // Create the messages background
    const auto textBackgroundTexture = sf::Texture::loadFromFile(graphicsContext, "resources/text-background.png").value();
    sf::Sprite textBackground(textBackgroundTexture.getRect());
    textBackground.setPosition({0.f, 520.f});
    textBackground.setColor(sf::Color(255, 255, 255, 200));

    // Create the description text
    sf::Text description(font, "Current effect: " + effectNames[current], 20);
    description.setPosition({10.f, 530.f});
    description.setFillColor(sf::Color(80, 80, 80));
    description.setOutlineThickness(3.f);
    description.setOutlineColor(sf::Color::Red);

    // Create the instructions text
    sf::Text instructions(font, "Press left and right arrows to change the current shader", 20);
    instructions.setPosition({280.f, 555.f});
    instructions.setFillColor(sf::Color(80, 80, 80));
    instructions.setOutlineThickness(3.f);
    instructions.setOutlineColor(sf::Color::Red);

    // Create the main window
    sf::RenderWindow window(graphicsContext, sf::VideoMode({800, 600}), "SFML Shader", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Start the game loop
    const sf::Clock clock;
    while (true)
    {
        // Process events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                switch (keyPressed->code)
                {
                    // Left arrow key: previous shader
                    case sf::Keyboard::Key::Left:
                    {
                        if (current == 0)
                            current = effects.size() - 1;
                        else
                            --current;

                        break;
                    }

                    // Right arrow key: next shader
                    case sf::Keyboard::Key::Right:
                    {
                        if (current == effects.size() - 1)
                            current = 0;
                        else
                            ++current;

                        break;
                    }

                    default:
                        break;
                }

                description.setString("Current effect: " + effectNames[current]);
            }
        }

        // If the current example was loaded successfully...
        if (Effect* currentEffect = effects[current])
        {
            // Update the current example
            const auto [x, y] = sf::Mouse::getPosition(window).to<sf::Vector2f>().cwiseDiv(
                window.getSize().to<sf::Vector2f>());
            currentEffect->update(clock.getElapsedTime().asSeconds(), x, y);

            // Clear the window
            window.clear(currentEffect == edgeEffect.asPtr() ? sf::Color::White : sf::Color(50, 50, 50));

            // Draw the current example
            window.draw(*currentEffect);
        }
        else
        {
            // Clear the window to grey to make sure the text is always readable
            window.clear(sf::Color(50, 50, 50));

            sf::Text error(font, "Shader not\nsupported", 36);
            error.setPosition({320.f, 200.f});

            window.draw(error);
        }

        // Draw the text
        window.draw(textBackground, textBackgroundTexture);
        window.draw(instructions);
        window.draw(description);

        // Finally, display the rendered frame on screen
        window.display();
    }
}
