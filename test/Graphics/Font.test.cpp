#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/FontInfo.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Graphics/Texture.hpp>

// Other 1st party headers
#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/Path.hpp>

#include <Doctest.hpp>

#include <GraphicsUtil.hpp>
#include <LoadIntoMemoryUtil.hpp>
#include <WindowUtil.hpp>

#include <type_traits>

TEST_CASE("[Graphics] sf::Font" * doctest::skip(skipDisplayTests))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_default_constructible_v<sf::Font>);
        STATIC_CHECK(std::is_copy_constructible_v<sf::Font>);
        STATIC_CHECK(std::is_copy_assignable_v<sf::Font>);
        STATIC_CHECK(std::is_move_constructible_v<sf::Font>);
        STATIC_CHECK(std::is_move_assignable_v<sf::Font>);
    }

    SECTION("openFromFile()")
    {
        SECTION("Invalid filename")
        {
            CHECK(!sf::Font::openFromFile(graphicsContext, "does/not/exist.ttf"));
        }

        SECTION("Valid file")
        {
            const auto font = sf::Font::openFromFile(graphicsContext, "Graphics/tuffy.ttf").value();
            CHECK(font.getInfo().family == "Tuffy");
            const auto& glyph = font.getGlyph(0x45, 16, false);
            CHECK(glyph.advance == 9);
            CHECK(glyph.lsbDelta == 9);
            CHECK(glyph.rsbDelta == 16);
            CHECK(glyph.bounds == sf::FloatRect({0, -12}, {8, 12}));
            CHECK(glyph.textureRect == sf::IntRect({2, 5}, {8, 12}));
            CHECK(font.hasGlyph(0x41));
            CHECK(font.hasGlyph(0xC0));
            CHECK(font.getKerning(0x41, 0x42, 12) == -1);
            CHECK(font.getKerning(0x43, 0x44, 24, true) == 0);
            CHECK(font.getLineSpacing(24) == 30);
            CHECK(font.getUnderlinePosition(36) == Approx(2.20312f));
            CHECK(font.getUnderlineThickness(48) == Approx(1.17188f));
            const auto& texture = font.getTexture(10);
            CHECK(texture.getSize() == sf::Vector2u{128, 128});
            CHECK(texture.isSmooth());
            CHECK(!texture.isSrgb());
            CHECK(!texture.isRepeated());
            CHECK(texture.getNativeHandle() != 0);
            CHECK(font.isSmooth());
        }
    }

    SECTION("openFromMemory()")
    {
        SECTION("Invalid data and size")
        {
            CHECK(!sf::Font::openFromMemory(graphicsContext, nullptr, 1));
            const std::byte testByte{0xCD};
            CHECK(!sf::Font::openFromMemory(graphicsContext, &testByte, 0));
        }

        SECTION("Valid data")
        {
            const auto memory = loadIntoMemory("Graphics/tuffy.ttf");
            const auto font   = sf::Font::openFromMemory(graphicsContext, memory.data(), memory.size()).value();
            CHECK(font.getInfo().family == "Tuffy");
            const auto& glyph = font.getGlyph(0x45, 16, false);
            CHECK(glyph.advance == 9);
            CHECK(glyph.lsbDelta == 9);
            CHECK(glyph.rsbDelta == 16);
            CHECK(glyph.bounds == sf::FloatRect({0, -12}, {8, 12}));
            CHECK(glyph.textureRect == sf::IntRect({2, 5}, {8, 12}));
            CHECK(font.hasGlyph(0x41));
            CHECK(font.hasGlyph(0xC0));
            CHECK(font.getKerning(0x41, 0x42, 12) == -1);
            CHECK(font.getKerning(0x43, 0x44, 24, true) == 0);
            CHECK(font.getLineSpacing(24) == 30);
            CHECK(font.getUnderlinePosition(36) == Approx(2.20312f));
            CHECK(font.getUnderlineThickness(48) == Approx(1.17188f));
            const auto& texture = font.getTexture(10);
            CHECK(texture.getSize() == sf::Vector2u{128, 128});
            CHECK(texture.isSmooth());
            CHECK(!texture.isSrgb());
            CHECK(!texture.isRepeated());
            CHECK(texture.getNativeHandle() != 0);
            CHECK(font.isSmooth());
        }
    }

    SECTION("openFromStream()")
    {
        auto       stream = sf::FileInputStream::open("Graphics/tuffy.ttf").value();
        const auto font   = sf::Font::openFromStream(graphicsContext, stream).value();
        CHECK(font.getInfo().family == "Tuffy");
        const auto& glyph = font.getGlyph(0x45, 16, false);
        CHECK(glyph.advance == 9);
        CHECK(glyph.lsbDelta == 9);
        CHECK(glyph.rsbDelta == 16);
        CHECK(glyph.bounds == sf::FloatRect({0, -12}, {8, 12}));
        CHECK(glyph.textureRect == sf::IntRect({2, 5}, {8, 12}));
        CHECK(font.hasGlyph(0x41));
        CHECK(font.hasGlyph(0xC0));
        CHECK(font.getKerning(0x41, 0x42, 12) == -1);
        CHECK(font.getKerning(0x43, 0x44, 24, true) == 0);
        CHECK(font.getLineSpacing(24) == 30);
        CHECK(font.getUnderlinePosition(36) == Approx(2.20312f));
        CHECK(font.getUnderlineThickness(48) == Approx(1.17188f));
        const auto& texture = font.getTexture(10);
        CHECK(texture.getSize() == sf::Vector2u{128, 128});
        CHECK(texture.isSmooth());
        CHECK(!texture.isSrgb());
        CHECK(!texture.isRepeated());
        CHECK(texture.getNativeHandle() != 0);
        CHECK(font.isSmooth());
    }

    SECTION("Set/get smooth")
    {
        auto font = sf::Font::openFromFile(graphicsContext, "Graphics/tuffy.ttf").value();
        font.setSmooth(false);
        CHECK(!font.isSmooth());
    }
}
