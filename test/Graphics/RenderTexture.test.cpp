#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <SFML/Window/GraphicsContext.hpp>

#include <Doctest.hpp>

#include <StringifyOptionalUtil.hpp>
#include <SystemUtil.hpp>
#include <WindowUtil.hpp>

#include <type_traits>

TEST_CASE("[Graphics] sf::RenderTexture" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_default_constructible_v<sf::RenderTexture>);
        STATIC_CHECK(!std::is_copy_constructible_v<sf::RenderTexture>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::RenderTexture>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::RenderTexture>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::RenderTexture>);
    }

    SECTION("create()")
    {
        CHECK(!sf::RenderTexture::create(graphicsContext, {1'000'000, 1'000'000}));

        CHECK(sf::RenderTexture::create(graphicsContext,
                                        {100, 100},
                                        sf::ContextSettings{8 /* depthBits */, 0 /* stencilBits */}));
        CHECK(sf::RenderTexture::create(graphicsContext,
                                        {100, 100},
                                        sf::ContextSettings{0 /* depthBits */, 8 /* stencilBits */}));

        const auto renderTexture = sf::RenderTexture::create(graphicsContext, {360, 480}).value();
        CHECK(renderTexture.getSize() == sf::Vector2u{360, 480});
        CHECK(!renderTexture.isSmooth());
        CHECK(!renderTexture.isRepeated());
        CHECK(!renderTexture.isSrgb());

        const auto& texture = renderTexture.getTexture();
        CHECK(texture.getSize() == sf::Vector2u{360, 480});
        CHECK(!texture.isSmooth());
        CHECK(!texture.isSrgb());
        CHECK(!texture.isRepeated());
        CHECK(texture.getNativeHandle() != 0);
    }
#if 0
    SECTION("getMaximumAntialiasingLevel()")
    {
        CHECK(sf::RenderTexture::getMaximumAntialiasingLevel(graphicsContext) <= 64);
    }

    SECTION("Set/get smooth")
    {
        auto renderTexture = sf::RenderTexture::create(graphicsContext, {64, 64}).value();
        renderTexture.setSmooth(true);
        CHECK(renderTexture.isSmooth());
    }

    SECTION("Set/get repeated")
    {
        auto renderTexture = sf::RenderTexture::create(graphicsContext, {64, 64}).value();
        renderTexture.setRepeated(true);
        CHECK(renderTexture.isRepeated());
    }

    SECTION("generateMipmap()")
    {
        auto renderTexture = sf::RenderTexture::create(graphicsContext, {64, 64}).value();
        CHECK(renderTexture.generateMipmap());
    }

    SECTION("setActive()")
    {
        auto renderTexture = sf::RenderTexture::create(graphicsContext, {64, 64}).value();
        CHECK(renderTexture.setActive());
        CHECK(renderTexture.setActive(false));
        CHECK(renderTexture.setActive(true));
    }

    SECTION("getTexture()")
    {
        const auto renderTexture = sf::RenderTexture::create(graphicsContext, {64, 64}).value();
        CHECK(renderTexture.getTexture().getSize() == sf::Vector2u{64, 64});
    }

    SECTION("Sanity check 1")
    {
        auto renderTexture = sf::RenderTexture::create(graphicsContext, {64, 64}, sf::ContextSettings(0, 0, 1)).value();

        renderTexture.clear(sf::Color::Green);
        renderTexture.display();

        auto image = renderTexture.getTexture().copyToImage();
        CHECK((image.getPixel({0u, 0u}) == sf::Color::Green));
    }

    SECTION("Sanity check 2")
    {
        const float width     = 128.f;
        const float height    = 64.f;
        const float halfWidth = width / 2.f;

        const sf::Vector2u size{static_cast<unsigned int>(width), static_cast<unsigned int>(height)};

        auto image   = sf::Image::create(size, sf::Color::White).value();
        auto texture = sf::Texture::loadFromImage(graphicsContext, image).value();

        auto baseRenderTexture = sf::RenderTexture::create(graphicsContext, size, sf::ContextSettings{0, 0, 4 /* AA level*/})
                                     .value();

        auto leftInnerRT = sf::RenderTexture::create(graphicsContext, size, sf::ContextSettings{0, 0, 4 /* AA level*/}).value();
        auto rightInnerRT = sf::RenderTexture::create(graphicsContext, size, sf::ContextSettings{0, 0, 4 /* AA level*/}).value();

        const sf::Vertex leftVertexArray[6]{{{0.f, 0.f}, sf::Color::White, {0.f, 0.f}},
                                            {{halfWidth, 0.f}, sf::Color::White, {halfWidth, 0.f}},
                                            {{0.f, height}, sf::Color::White, {0.f, height}},
                                            {{0.f, height}, sf::Color::White, {0.f, height}},
                                            {{halfWidth, 0.f}, sf::Color::White, {halfWidth, 0.f}},
                                            {{halfWidth, height}, sf::Color::White, {halfWidth, height}}};

        const sf::Vertex rightVertexArray[6]{{{halfWidth, 0.f}, sf::Color::White, {halfWidth, 0.f}},
                                             {{width, 0.f}, sf::Color::White, {width, 0.f}},
                                             {{halfWidth, height}, sf::Color::White, {halfWidth, height}},
                                             {{halfWidth, height}, sf::Color::White, {halfWidth, height}},
                                             {{width, 0.f}, sf::Color::White, {width, 0.f}},
                                             {{width, height}, sf::Color::White, {width, height}}};

        leftInnerRT.clear();
        rightInnerRT.clear();

        sf::Sprite sprite(texture.getRect());
        leftInnerRT.draw(sprite, texture);

        sprite.setColor(sf::Color::Green);
        rightInnerRT.draw(sprite, texture);

        baseRenderTexture.clear();

        leftInnerRT.display();
        baseRenderTexture.draw(leftVertexArray, sf::PrimitiveType::Triangles, sf::RenderStates{&leftInnerRT.getTexture()});

        rightInnerRT.display();
        baseRenderTexture.draw(rightVertexArray, sf::PrimitiveType::Triangles, sf::RenderStates{&rightInnerRT.getTexture()});

        baseRenderTexture.display();

        auto finalImage = baseRenderTexture.getTexture().copyToImage();

        CHECK(finalImage.getSize() == size);
        CHECK((finalImage.getPixel({0u, 0u}) == sf::Color::White));
        CHECK((finalImage.getPixel({static_cast<unsigned int>(width / 2.f) + 1u, 0u}) == sf::Color::Green));
    }
#endif
}
