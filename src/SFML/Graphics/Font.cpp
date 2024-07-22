////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/FontInfo.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Window/GraphicsContext.hpp>
#ifdef SFML_SYSTEM_ANDROID
#include <SFML/System/Android/ResourceStream.hpp>
#endif
#include <SFML/System/Err.hpp>
#include <SFML/System/InputStream.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/PathUtils.hpp>

#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Math.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_STROKER_H

#include <SFML/Base/Assert.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

#include <cstring>


namespace
{
// FreeType callbacks that operate on a sf::InputStream
unsigned long read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    auto* stream = static_cast<sf::InputStream*>(rec->descriptor.pointer);
    if (sf::base::Optional seekResult = stream->seek(offset); seekResult.hasValue() && *seekResult == offset)
    {
        if (count > 0)
            return static_cast<unsigned long>(stream->read(reinterpret_cast<char*>(buffer), count).value());

        return 0;
    }

    return count > 0 ? 0 : 1; // error code is 0 if we're reading, or nonzero if we're seeking
}
void close(FT_Stream)
{
}

// Helper to interpret memory as a specific type
template <typename T, typename U>
inline T reinterpret(const U& input)
{
    T output;
    std::memcpy(&output, &input, sizeof(U));
    return output;
}

// Combine outline thickness, boldness and font glyph index into a single 64-bit key
std::uint64_t combine(float outlineThickness, bool bold, std::uint32_t index)
{
    return (std::uint64_t{reinterpret<std::uint32_t>(outlineThickness)} << 32) | (std::uint64_t{bold} << 31) | index;
}
} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Font::Page
{
    struct [[nodiscard]] Row
    {
        Row(unsigned int rowTop, unsigned int rowHeight) : top(rowTop), height(rowHeight)
        {
        }

        unsigned int width{}; //!< Current width of the row
        unsigned int top;     //!< Y position of the row into the texture
        unsigned int height;  //!< Height of the row
    };

    using GlyphTable = std::unordered_map<std::uint64_t, Glyph>; //!< Table mapping a codepoint to its glyph

    [[nodiscard]] static base::Optional<Page> create(GraphicsContext& graphicsContext, bool smooth);
    explicit Page(Texture&& texture);

    GlyphTable       glyphs;     //!< Table mapping code points to their corresponding glyph
    Texture          texture;    //!< Texture containing the pixels of the glyphs
    unsigned int     nextRow{3}; //!< Y position of the next new row in the texture
    std::vector<Row> rows;       //!< List containing the position of all the existing rows
};


////////////////////////////////////////////////////////////
struct Font::FontHandles
{
    FontHandles() = default;

    ~FontHandles()
    {
        // All the function below are safe to call with null pointer arguments.
        // The documentation of FreeType isn't clear on the matter, but the
        // implementation does explicitly check for null.

        FT_Stroker_Done(stroker);
        FT_Done_Face(face);
        // `streamRec` doesn't need to be explicitly freed.
        FT_Done_FreeType(library);
    }

    // clang-format off
    FontHandles(const FontHandles&)            = delete;
    FontHandles& operator=(const FontHandles&) = delete;

    FontHandles(FontHandles&&)            = delete;
    FontHandles& operator=(FontHandles&&) = delete;
    // clang-format on

    FT_Library   library{};   //< Pointer to the internal library interface
    FT_StreamRec streamRec{}; //< Stream rec object describing an input stream
    FT_Face      face{};      //< Pointer to the internal font face
    FT_Stroker   stroker{};   //< Pointer to the stroker
};


////////////////////////////////////////////////////////////
struct Font::Impl
{
    explicit Impl(GraphicsContext& theGraphicsContext, std::shared_ptr<FontHandles>&& theFontHandles, const char* theFamilyName) :
    graphicsContext(&theGraphicsContext),
    fontHandles(SFML_BASE_MOVE(theFontHandles)),
    info{theFamilyName}
    {
    }

    using PageTable = std::unordered_map<unsigned int, Page>; //!< Table mapping a character size to its page (texture)

    GraphicsContext*             graphicsContext; //!< The graphics context
    std::shared_ptr<FontHandles> fontHandles;     //!< Shared information about the internal font instance
    bool                         isSmooth{true};  //!< Status of the smooth filter
    FontInfo                     info;            //!< Information about the font
    mutable PageTable            pages;           //!< Table containing the glyphs pages by character size
    mutable std::vector<std::uint8_t> pixelBuffer; //!< Pixel buffer holding a glyph's pixels before being written to the texture
#ifdef SFML_SYSTEM_ANDROID
    base::UniquePtr<priv::ResourceStream> m_stream; //!< Asset file streamer (if loaded from file)
#endif
};


////////////////////////////////////////////////////////////
Font::Font(base::PassKey<Font>&&, GraphicsContext& graphicsContext, void* fontHandlesSharedPtr, const char* familyName) :
m_impl(graphicsContext, SFML_BASE_MOVE(*static_cast<std::shared_ptr<FontHandles>*>(fontHandlesSharedPtr)), familyName)
{
}


////////////////////////////////////////////////////////////
Font::~Font() = default;


////////////////////////////////////////////////////////////
Font::Font(const Font&) = default;


////////////////////////////////////////////////////////////
Font::Font(Font&&) noexcept = default;


////////////////////////////////////////////////////////////
Font& Font::operator=(const Font&) = default;


////////////////////////////////////////////////////////////
Font& Font::operator=(Font&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromFile(GraphicsContext& graphicsContext, const Path& filename)
{
#ifndef SFML_SYSTEM_ANDROID

    auto fontHandles = std::make_shared<FontHandles>();

    // Initialize FreeType
    // Note: we initialize FreeType for every font instance in order to avoid having a single
    // global manager that would create a lot of issues regarding creation and destruction order.
    if (FT_Init_FreeType(&fontHandles->library) != 0)
    {
        priv::err() << "Failed to load font (failed to initialize FreeType)\n" << priv::PathDebugFormatter{filename};
        return base::nullOpt;
    }

    // Load the new font face from the specified file
    FT_Face face = nullptr;
    if (FT_New_Face(fontHandles->library, filename.to<std::string>().c_str(), 0, &face) != 0)
    {
        priv::err() << "Failed to load font (failed to create the font face)\n" << priv::PathDebugFormatter{filename};
        return base::nullOpt;
    }
    fontHandles->face = face;

    // Load the stroker that will be used to outline the font
    if (FT_Stroker_New(fontHandles->library, &fontHandles->stroker) != 0)
    {
        priv::err() << "Failed to load font (failed to create the stroker)\n" << priv::PathDebugFormatter{filename};
        return base::nullOpt;
    }

    // Select the unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
    {
        priv::err() << "Failed to load font (failed to set the Unicode character set)\n"
                    << priv::PathDebugFormatter{filename};
        return base::nullOpt;
    }

    return sf::base::makeOptional<Font>(base::PassKey<Font>{}, graphicsContext, &fontHandles, face->family_name);

#else

    auto stream = base::makeUnique<priv::ResourceStream>(filename);
    auto font   = openFromStream(*stream);
    if (font)
        font->m_stream = SFML_BASE_MOVE(stream);
    return font;

#endif
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromMemory(GraphicsContext& graphicsContext, const void* data, std::size_t sizeInBytes)
{
    auto fontHandles = std::make_shared<FontHandles>();

    // Initialize FreeType
    // Note: we initialize FreeType for every font instance in order to avoid having a single
    // global manager that would create a lot of issues regarding creation and destruction order.
    if (FT_Init_FreeType(&fontHandles->library) != 0)
    {
        priv::err() << "Failed to load font from memory (failed to initialize FreeType)";
        return base::nullOpt;
    }

    // Load the new font face from the specified file
    FT_Face face = nullptr;
    if (FT_New_Memory_Face(fontHandles->library,
                           reinterpret_cast<const FT_Byte*>(data),
                           static_cast<FT_Long>(sizeInBytes),
                           0,
                           &face) != 0)
    {
        priv::err() << "Failed to load font from memory (failed to create the font face)";
        return base::nullOpt;
    }
    fontHandles->face = face;

    // Load the stroker that will be used to outline the font
    if (FT_Stroker_New(fontHandles->library, &fontHandles->stroker) != 0)
    {
        priv::err() << "Failed to load font from memory (failed to create the stroker)";
        return base::nullOpt;
    }

    // Select the Unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
    {
        priv::err() << "Failed to load font from memory (failed to set the Unicode character set)";
        return base::nullOpt;
    }

    return sf::base::makeOptional<Font>(base::PassKey<Font>{}, graphicsContext, &fontHandles, face->family_name);
}


////////////////////////////////////////////////////////////
base::Optional<Font> Font::openFromStream(GraphicsContext& graphicsContext, InputStream& stream)
{
    auto fontHandles = std::make_shared<FontHandles>();

    // Initialize FreeType
    // Note: we initialize FreeType for every font instance in order to avoid having a single
    // global manager that would create a lot of issues regarding creation and destruction order.
    if (FT_Init_FreeType(&fontHandles->library) != 0)
    {
        priv::err() << "Failed to load font from stream (failed to initialize FreeType)";
        return base::nullOpt;
    }

    // Make sure that the stream's reading position is at the beginning
    if (!stream.seek(0).hasValue())
    {
        priv::err() << "Failed to seek font stream";
        return base::nullOpt;
    }

    // Prepare a wrapper for our stream, that we'll pass to FreeType callbacks
    fontHandles->streamRec.base               = nullptr;
    fontHandles->streamRec.size               = static_cast<unsigned long>(stream.getSize().value());
    fontHandles->streamRec.pos                = 0;
    fontHandles->streamRec.descriptor.pointer = &stream;
    fontHandles->streamRec.read               = &read;
    fontHandles->streamRec.close              = &close;

    // Setup the FreeType callbacks that will read our stream
    FT_Open_Args args;
    args.flags  = FT_OPEN_STREAM;
    args.stream = &fontHandles->streamRec;
    args.driver = nullptr;

    // Load the new font face from the specified stream
    FT_Face face = nullptr;
    if (FT_Open_Face(fontHandles->library, &args, 0, &face) != 0)
    {
        priv::err() << "Failed to load font from stream (failed to create the font face)";
        return base::nullOpt;
    }
    fontHandles->face = face;

    // Load the stroker that will be used to outline the font
    if (FT_Stroker_New(fontHandles->library, &fontHandles->stroker) != 0)
    {
        priv::err() << "Failed to load font from stream (failed to create the stroker)";
        return base::nullOpt;
    }

    // Select the Unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
    {
        priv::err() << "Failed to load font from stream (failed to set the Unicode character set)";
        return base::nullOpt;
    }

    return sf::base::makeOptional<Font>(base::PassKey<Font>{}, graphicsContext, &fontHandles, face->family_name);
}


////////////////////////////////////////////////////////////
const FontInfo& Font::getInfo() const
{
    return m_impl->info;
}


////////////////////////////////////////////////////////////
unsigned int Font::getCharIndex(std::uint32_t codePoint) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);
    return FT_Get_Char_Index(m_impl->fontHandles->face, codePoint);
}


////////////////////////////////////////////////////////////
const Glyph& Font::getGlyph(std::uint32_t codePoint, unsigned int characterSize, bool bold, float outlineThickness) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    // Get the page corresponding to the character size
    Page::GlyphTable& glyphs = loadPage(*m_impl->graphicsContext, characterSize).glyphs;

    // Build the key by combining the glyph index (based on code point), bold flag, and outline thickness
    const std::uint64_t key = combine(outlineThickness, bold, getCharIndex(codePoint));

    // Search the glyph into the cache
    if (const auto it = glyphs.find(key); it != glyphs.end())
    {
        // Found: just return it
        return it->second;
    }

    // Not found: we have to load it
    const Glyph glyph = loadGlyph(codePoint, characterSize, bold, outlineThickness);
    return glyphs.emplace(key, glyph).first->second;
}


////////////////////////////////////////////////////////////
bool Font::hasGlyph(std::uint32_t codePoint) const
{
    return getCharIndex(codePoint) != 0;
}


////////////////////////////////////////////////////////////
float Font::getKerning(std::uint32_t first, std::uint32_t second, unsigned int characterSize, bool bold) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    // Special case where first or second is 0 (null character)
    if (first == 0 || second == 0)
        return 0.f;

    FT_Face face = m_impl->fontHandles->face;

    if (face && setCurrentSize(characterSize))
    {
        // Convert the characters to indices
        const FT_UInt index1 = FT_Get_Char_Index(face, first);
        const FT_UInt index2 = FT_Get_Char_Index(face, second);

        // Retrieve position compensation deltas generated by FT_LOAD_FORCE_AUTOHINT flag
        const auto firstRsbDelta  = static_cast<float>(getGlyph(first, characterSize, bold).rsbDelta);
        const auto secondLsbDelta = static_cast<float>(getGlyph(second, characterSize, bold).lsbDelta);

        // Get the kerning vector if present
        FT_Vector kerning{0, 0};
        if (FT_HAS_KERNING(face))
            FT_Get_Kerning(face, index1, index2, FT_KERNING_UNFITTED, &kerning);

        // X advance is already in pixels for bitmap fonts
        if (!FT_IS_SCALABLE(face))
            return static_cast<float>(kerning.x);

        // Combine kerning with compensation deltas and return the X advance
        // Flooring is required as we use FT_KERNING_UNFITTED flag which is not quantized in 64 based grid
        return base::floor((secondLsbDelta - firstRsbDelta + static_cast<float>(kerning.x) + 32) / float{1 << 6});
    }

    // Invalid font
    return 0.f;
}


////////////////////////////////////////////////////////////
float Font::getLineSpacing(unsigned int characterSize) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    FT_Face face = m_impl->fontHandles->face;

    if (setCurrentSize(characterSize))
    {
        return static_cast<float>(face->size->metrics.height) / float{1 << 6};
    }

    return 0.f;
}


////////////////////////////////////////////////////////////
float Font::getUnderlinePosition(unsigned int characterSize) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    FT_Face face = m_impl->fontHandles->face;

    if (setCurrentSize(characterSize))
    {
        // Return a fixed position if font is a bitmap font
        if (!FT_IS_SCALABLE(face))
            return static_cast<float>(characterSize) / 10.f;

        return -static_cast<float>(FT_MulFix(face->underline_position, face->size->metrics.y_scale)) / float{1 << 6};
    }

    return 0.f;
}


////////////////////////////////////////////////////////////
float Font::getUnderlineThickness(unsigned int characterSize) const
{
    SFML_BASE_ASSERT(m_impl->fontHandles != nullptr);

    FT_Face face = m_impl->fontHandles->face;

    if (face && setCurrentSize(characterSize))
    {
        // Return a fixed thickness if font is a bitmap font
        if (!FT_IS_SCALABLE(face))
            return static_cast<float>(characterSize) / 14.f;

        return static_cast<float>(FT_MulFix(face->underline_thickness, face->size->metrics.y_scale)) / float{1 << 6};
    }

    return 0.f;
}


////////////////////////////////////////////////////////////
const Texture& Font::getTexture(unsigned int characterSize) const
{
    return loadPage(*m_impl->graphicsContext, characterSize).texture;
}

////////////////////////////////////////////////////////////
void Font::setSmooth(bool smooth)
{
    if (smooth != m_impl->isSmooth)
    {
        m_impl->isSmooth = smooth;

        for (auto& [key, page] : m_impl->pages)
        {
            page.texture.setSmooth(m_impl->isSmooth);
        }
    }
}

////////////////////////////////////////////////////////////
bool Font::isSmooth() const
{
    return m_impl->isSmooth;
}


////////////////////////////////////////////////////////////
Font::Page& Font::loadPage(GraphicsContext& graphicsContext, unsigned int characterSize) const
{
    if (const auto it = m_impl->pages.find(characterSize); it != m_impl->pages.end())
        return it->second;

    auto page = Page::create(graphicsContext, m_impl->isSmooth);
    SFML_BASE_ASSERT(page.hasValue() && "Font::loadPage() Failed to load page");

    return m_impl->pages.emplace(characterSize, SFML_BASE_MOVE(*page)).first->second;
}


////////////////////////////////////////////////////////////
Glyph Font::loadGlyph(std::uint32_t codePoint, unsigned int characterSize, bool bold, float outlineThickness) const
{
    // The glyph to return
    Glyph glyph;

    // Get our FT_Face
    FT_Face face = m_impl->fontHandles->face;
    if (!face)
        return glyph;

    // Set the character size
    if (!setCurrentSize(characterSize))
        return glyph;

    // Load the glyph corresponding to the code point
    FT_Int32 flags = FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT;
    if (outlineThickness != 0)
        flags |= FT_LOAD_NO_BITMAP;
    if (FT_Load_Char(face, codePoint, flags) != 0)
        return glyph;

    // Retrieve the glyph
    FT_Glyph glyphDesc = nullptr;
    if (FT_Get_Glyph(face->glyph, &glyphDesc) != 0)
        return glyph;

    // Apply bold and outline (there is no fallback for outline) if necessary -- first technique using outline (highest quality)
    const FT_Pos weight  = 1 << 6;
    const bool   outline = (glyphDesc->format == FT_GLYPH_FORMAT_OUTLINE);
    if (outline)
    {
        if (bold)
        {
            auto* outlineGlyph = reinterpret_cast<FT_OutlineGlyph>(glyphDesc);
            FT_Outline_Embolden(&outlineGlyph->outline, weight);
        }

        if (outlineThickness != 0)
        {
            FT_Stroker stroker = m_impl->fontHandles->stroker;

            FT_Stroker_Set(stroker,
                           static_cast<FT_Fixed>(outlineThickness * float{1 << 6}),
                           FT_STROKER_LINECAP_ROUND,
                           FT_STROKER_LINEJOIN_ROUND,
                           0);
            FT_Glyph_Stroke(&glyphDesc, stroker, true);
        }
    }

    // Convert the glyph to a bitmap (i.e. rasterize it)
    // Warning! After this line, do not read any data from glyphDesc directly, use
    // bitmapGlyph.root to access the FT_Glyph data.
    FT_Glyph_To_Bitmap(&glyphDesc, FT_RENDER_MODE_NORMAL, nullptr, 1);
    auto*      bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyphDesc);
    FT_Bitmap& bitmap      = bitmapGlyph->bitmap;

    // Apply bold if necessary -- fallback technique using bitmap (lower quality)
    if (!outline)
    {
        if (bold)
            FT_Bitmap_Embolden(m_impl->fontHandles->library, &bitmap, weight, weight);

        if (outlineThickness != 0)
            priv::err() << "Failed to outline glyph (no fallback available)";
    }

    // Compute the glyph's advance offset
    glyph.advance = static_cast<float>(bitmapGlyph->root.advance.x >> 16);
    if (bold)
        glyph.advance += static_cast<float>(weight) / float{1 << 6};

    glyph.lsbDelta = static_cast<int>(face->glyph->lsb_delta);
    glyph.rsbDelta = static_cast<int>(face->glyph->rsb_delta);

    Vector2u size(bitmap.width, bitmap.rows);

    if ((size.x > 0) && (size.y > 0))
    {
        // Leave a small padding around characters, so that filtering doesn't
        // pollute them with pixels from neighbors
        const unsigned int padding = 2;

        size += 2u * Vector2u{padding, padding};

        // Get the glyphs page corresponding to the character size
        Page& page = loadPage(*m_impl->graphicsContext, characterSize);

        // Find a good position for the new glyph into the texture
        glyph.textureRect = findGlyphRect(*m_impl->graphicsContext, page, size);

        // Make sure the texture data is positioned in the center
        // of the allocated texture rectangle
        glyph.textureRect.position += Vector2i{padding, padding};
        glyph.textureRect.size -= 2 * Vector2i{padding, padding};

        // Compute the glyph's bounding box
        glyph.bounds.position = Vector2i(bitmapGlyph->left, -bitmapGlyph->top).to<Vector2f>();
        glyph.bounds.size     = Vector2u(bitmap.width, bitmap.rows).to<Vector2f>();

        // Resize the pixel buffer to the new size and fill it with transparent white pixels
        m_impl->pixelBuffer.resize(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y) * 4);

        std::uint8_t* current = m_impl->pixelBuffer.data();
        std::uint8_t* end     = current + size.x * size.y * 4;

        while (current != end)
        {
            (*current++) = 255;
            (*current++) = 255;
            (*current++) = 255;
            (*current++) = 0;
        }

        // Extract the glyph's pixels from the bitmap
        const std::uint8_t* pixels = bitmap.buffer;
        if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
        {
            // Pixels are 1 bit monochrome values
            for (unsigned int y = padding; y < size.y - padding; ++y)
            {
                for (unsigned int x = padding; x < size.x - padding; ++x)
                {
                    // The color channels remain white, just fill the alpha channel
                    const std::size_t index = x + y * size.x;
                    m_impl->pixelBuffer[index * 4 + 3] = ((pixels[(x - padding) / 8]) & (1 << (7 - ((x - padding) % 8))))
                                                             ? 255
                                                             : 0;
                }
                pixels += bitmap.pitch;
            }
        }
        else
        {
            // Pixels are 8 bit gray levels
            for (unsigned int y = padding; y < size.y - padding; ++y)
            {
                for (unsigned int x = padding; x < size.x - padding; ++x)
                {
                    // The color channels remain white, just fill the alpha channel
                    const std::size_t index            = x + y * size.x;
                    m_impl->pixelBuffer[index * 4 + 3] = pixels[x - padding];
                }
                pixels += bitmap.pitch;
            }
        }

        // Write the pixels to the texture
        const auto dest       = glyph.textureRect.position.to<Vector2u>() - Vector2u{padding, padding};
        const auto updateSize = glyph.textureRect.size.to<Vector2u>() + 2u * Vector2u{padding, padding};
        page.texture.update(m_impl->pixelBuffer.data(), updateSize, dest);
    }

    // Delete the FT glyph
    FT_Done_Glyph(glyphDesc);

    // Done :)
    return glyph;
}


////////////////////////////////////////////////////////////
IntRect Font::findGlyphRect(GraphicsContext& graphicsContext, Page& page, Vector2u size) const
{
    // Find the line that fits well the glyph
    Page::Row* row       = nullptr;
    float      bestRatio = 0;
    for (auto it = page.rows.begin(); it != page.rows.end() && !row; ++it)
    {
        const float ratio = static_cast<float>(size.y) / static_cast<float>(it->height);

        // Ignore rows that are either too small or too high
        if ((ratio < 0.7f) || (ratio > 1.f))
            continue;

        // Check if there's enough horizontal space left in the row
        if (size.x > page.texture.getSize().x - it->width)
            continue;

        // Make sure that this new row is the best found so far
        if (ratio < bestRatio)
            continue;

        // The current row passed all the tests: we can select it
        row       = &*it;
        bestRatio = ratio;
    }

    IntRect rect{{0, 0}, {2, 2}}; // Use a single local variable for NRVO

    // If we didn't find a matching row, create a new one (10% taller than the glyph)
    if (!row)
    {
        const unsigned int rowHeight = size.y + size.y / 10;
        while ((page.nextRow + rowHeight >= page.texture.getSize().y) || (size.x >= page.texture.getSize().x))
        {
            // Not enough space: resize the texture if possible
            const Vector2u textureSize = page.texture.getSize();
            if ((textureSize.x * 2 <= Texture::getMaximumSize(graphicsContext)) &&
                (textureSize.y * 2 <= Texture::getMaximumSize(graphicsContext)))
            {
                // Make the texture 2 times bigger
                auto newTexture = sf::Texture::create(graphicsContext, textureSize * 2u);
                if (!newTexture)
                {
                    priv::err() << "Failed to create new page texture";
                    return rect;
                }

                newTexture->setSmooth(m_impl->isSmooth);
                newTexture->update(page.texture);
                page.texture.swap(*newTexture);
            }
            else
            {
                // Oops, we've reached the maximum texture size...
                priv::err() << "Failed to add a new character to the font: the maximum texture size has been reached";
                return rect;
            }
        }

        // We can now create the new row
        page.rows.emplace_back(page.nextRow, rowHeight);
        page.nextRow += rowHeight;
        row = &page.rows.back();
    }

    // Find the glyph's rectangle on the selected row
    rect.position.x = static_cast<int>(row->width);
    rect.position.y = static_cast<int>(row->top);
    rect.size.x     = static_cast<int>(size.x);
    rect.size.y     = static_cast<int>(size.y);

    // Update the row information
    row->width += size.x;

    return rect;
}


////////////////////////////////////////////////////////////
bool Font::setCurrentSize(unsigned int characterSize) const
{
    // FT_Set_Pixel_Sizes is an expensive function, so we must call it
    // only when necessary to avoid killing performances

    // fontHandles and fontHandles->face are checked to be non-null before calling this method
    FT_Face         face        = m_impl->fontHandles->face;
    const FT_UShort currentSize = face->size->metrics.x_ppem;

    if (currentSize == characterSize)
        return true;

    const FT_Error result = FT_Set_Pixel_Sizes(face, 0, characterSize);

    if (result != FT_Err_Invalid_Pixel_Size)
        return result == FT_Err_Ok;

    // In the case of bitmap fonts, resizing can fail if the requested size is not available
    if (!FT_IS_SCALABLE(face))
    {
        priv::err(true /* multiLine */) << "Failed to set bitmap font size to " << characterSize << '\n'
                                        << "Available sizes are: ";

        for (int i = 0; i < face->num_fixed_sizes; ++i)
        {
            const long size = (face->available_sizes[i].y_ppem + 32) >> 6;
            priv::err(true /* multiLine */) << size << " ";
        }

        priv::err() << '\n';
    }
    else
    {
        priv::err() << "Failed to set font size to " << characterSize;
    }

    return false;
}


////////////////////////////////////////////////////////////
base::Optional<Font::Page> Font::Page::create(GraphicsContext& graphicsContext, bool smooth)
{
    // Make sure that the texture is initialized by default
    auto image = *Image::create({128, 128}, Color::Transparent);

    // Reserve a 2x2 white square for texturing underlines
    for (unsigned int x = 0; x < 2; ++x)
        for (unsigned int y = 0; y < 2; ++y)
            image.setPixel({x, y}, Color::White);

    // Create the texture
    auto texture = sf::Texture::loadFromImage(graphicsContext, image);
    if (!texture.hasValue())
    {
        priv::err() << "Failed to load font page texture";
        return base::nullOpt;
    }

    texture->setSmooth(smooth);
    return sf::base::makeOptional<Page>(SFML_BASE_MOVE(*texture));
}


////////////////////////////////////////////////////////////
Font::Page::Page(Texture&& theTexture) : texture(SFML_BASE_MOVE(theTexture))
{
}

} // namespace sf
