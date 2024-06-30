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
#define MINIMP3_IMPLEMENTATION // Minimp3 control define, places implementation in this file.
#ifndef NOMINMAX
#define NOMINMAX // To avoid windows.h and min/max issue
#endif
#define MINIMP3_NO_STDIO // Minimp3 control define, eliminate file manipulation code which is useless here

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4242 4244 4267 4456 4706)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

#include <minimp3_ex.h>

#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

#undef NOMINMAX
#undef MINIMP3_NO_STDIO

#include <SFML/Audio/SoundFileReaderMp3.hpp>

#include <SFML/System/AlgorithmUtils.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/System/InputStream.hpp>

#include <cassert>
#include <cstdint>
#include <cstring>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] std::size_t readCallback(void* ptr, std::size_t size, void* data)
{
    auto* stream = static_cast<sf::InputStream*>(data);
    return stream->read(ptr, size).value_or(-1);
}


////////////////////////////////////////////////////////////
[[nodiscard]] int seekCallback(std::uint64_t offset, void* data)
{
    auto*               stream   = static_cast<sf::InputStream*>(data);
    const std::optional position = stream->seek(static_cast<std::size_t>(offset));
    return position ? 0 : -1;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool hasValidId3Tag(const std::uint8_t* header)
{
    return std::memcmp(header, "ID3", 3) == 0 &&
           !((header[5] & 15) || (header[6] & 0x80) || (header[7] & 0x80) || (header[8] & 0x80) || (header[9] & 0x80));
}

} // namespace

namespace sf::priv
{
////////////////////////////////////////////////////////////
struct SoundFileReaderMp3::Impl
{
    mp3dec_io_t   io{};
    mp3dec_ex_t   decoder{};
    std::uint64_t numSamples{}; // Decompressed audio storage size
    std::uint64_t position{};   // Position in decompressed audio buffer
};


////////////////////////////////////////////////////////////
bool SoundFileReaderMp3::check(InputStream& stream)
{
    std::uint8_t header[10];

    if (stream.read(header, sizeof(header)) != sizeof(header))
        return false;

    if (hasValidId3Tag(header))
        return true;

    if (hdr_valid(header))
        return true;

    return false;
}


////////////////////////////////////////////////////////////
SoundFileReaderMp3::SoundFileReaderMp3()
{
    m_impl->io.read = readCallback;
    m_impl->io.seek = seekCallback;
}


////////////////////////////////////////////////////////////
SoundFileReaderMp3::~SoundFileReaderMp3()
{
    mp3dec_ex_close(&m_impl->decoder);
}


////////////////////////////////////////////////////////////
std::optional<SoundFileReader::Info> SoundFileReaderMp3::open(InputStream& stream)
{
    // Init IO callbacks
    m_impl->io.read_data = &stream;
    m_impl->io.seek_data = &stream;

    std::optional<Info> result; // Use a single local variable for NRVO

    // Init mp3 decoder
    mp3dec_ex_open_cb(&m_impl->decoder, &m_impl->io, MP3D_SEEK_TO_SAMPLE);
    if (!m_impl->decoder.samples)
        return result; // Empty optional

    // Retrieve the music attributes
    Info& info        = result.emplace();
    info.channelCount = static_cast<unsigned int>(m_impl->decoder.info.channels);
    info.sampleRate   = static_cast<unsigned int>(m_impl->decoder.info.hz);
    info.sampleCount  = m_impl->decoder.samples;

    // MP3 only supports mono/stereo channels
    switch (info.channelCount)
    {
        case 0:
            priv::err() << "No channels in MP3 file" << priv::errEndl;
            break;
        case 1:
            info.channelMap = {SoundChannel::Mono};
            break;
        case 2:
            info.channelMap = {SoundChannel::SideLeft, SoundChannel::SideRight};
            break;
        default:
            priv::err() << "MP3 files with more than 2 channels not supported" << priv::errEndl;
            assert(false);
            break;
    }

    m_impl->numSamples = info.sampleCount;
    return result;
}


////////////////////////////////////////////////////////////
void SoundFileReaderMp3::seek(std::uint64_t sampleOffset)
{
    m_impl->position = priv::min(sampleOffset, m_impl->numSamples);
    mp3dec_ex_seek(&m_impl->decoder, m_impl->position);
}


////////////////////////////////////////////////////////////
std::uint64_t SoundFileReaderMp3::read(std::int16_t* samples, std::uint64_t maxCount)
{
    std::uint64_t toRead = priv::min(maxCount, m_impl->numSamples - m_impl->position);
    toRead               = std::uint64_t{mp3dec_ex_read(&m_impl->decoder, samples, static_cast<std::size_t>(toRead))};
    m_impl->position += toRead;
    return toRead;
}

} // namespace sf::priv
