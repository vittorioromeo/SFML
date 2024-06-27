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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/Export.hpp>

#include <SFML/Audio/SoundChannel.hpp>

#include <SFML/System/LifetimeTracking.hpp>
#include <SFML/System/PassKey.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/UniquePtr.hpp>

#include <filesystem>
#include <optional>
#include <vector>

#include <cstddef>
#include <cstdint>


namespace sf
{
class Sound;
class InputSoundFile;
class InputStream;

////////////////////////////////////////////////////////////
/// \brief Storage for audio samples defining a sound
///
////////////////////////////////////////////////////////////
class SFML_AUDIO_API SoundBuffer
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    /// \param copy Instance to copy
    ///
    ////////////////////////////////////////////////////////////
    SoundBuffer(const SoundBuffer& copy);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SoundBuffer();

    ////////////////////////////////////////////////////////////
    /// \brief Load the sound buffer from a file
    ///
    /// See the documentation of sf::InputSoundFile for the list
    /// of supported formats.
    ///
    /// \param filename Path of the sound file to load
    ///
    /// \return Sound buffer if loading succeeded, `std::nullopt` if it failed
    ///
    /// \see loadFromMemory, loadFromStream, loadFromSamples, saveToFile
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<SoundBuffer> loadFromFile(const std::filesystem::path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Load the sound buffer from a file in memory
    ///
    /// See the documentation of sf::InputSoundFile for the list
    /// of supported formats.
    ///
    /// \param data        Pointer to the file data in memory
    /// \param sizeInBytes Size of the data to load, in bytes
    ///
    /// \return Sound buffer if loading succeeded, `std::nullopt` if it failed
    ///
    /// \see loadFromFile, loadFromStream, loadFromSamples
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<SoundBuffer> loadFromMemory(const void* data, std::size_t sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Load the sound buffer from a custom stream
    ///
    /// See the documentation of sf::InputSoundFile for the list
    /// of supported formats.
    ///
    /// \param stream Source stream to read from
    ///
    /// \return Sound buffer if loading succeeded, `std::nullopt` if it failed
    ///
    /// \see loadFromFile, loadFromMemory, loadFromSamples
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<SoundBuffer> loadFromStream(InputStream& stream);

    ////////////////////////////////////////////////////////////
    /// \brief Load the sound buffer from an array of audio samples
    ///
    /// The assumed format of the audio samples is 16 bits signed integer.
    ///
    /// \param samples      Pointer to the array of samples in memory
    /// \param sampleCount  Number of samples in the array
    /// \param channelCount Number of channels (1 = mono, 2 = stereo, ...)
    /// \param sampleRate   Sample rate (number of samples to play per second)
    /// \param channelMap   Map of position in sample frame to sound channel
    ///
    /// \return Sound buffer if loading succeeded, `std::nullopt` if it failed
    ///
    /// \see loadFromFile, loadFromMemory, saveToFile
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<SoundBuffer> loadFromSamples(
        const std::int16_t*              samples,
        std::uint64_t                    sampleCount,
        unsigned int                     channelCount,
        unsigned int                     sampleRate,
        const std::vector<SoundChannel>& channelMap);

    ////////////////////////////////////////////////////////////
    /// \brief Save the sound buffer to an audio file
    ///
    /// See the documentation of sf::OutputSoundFile for the list
    /// of supported formats.
    ///
    /// \param filename Path of the sound file to write
    ///
    /// \return True if saving succeeded, false if it failed
    ///
    /// \see loadFromFile, loadFromMemory, loadFromSamples
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool saveToFile(const std::filesystem::path& filename) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the array of audio samples stored in the buffer
    ///
    /// The format of the returned samples is 16 bits signed integer.
    /// The total number of samples in this array is given by the
    /// getSampleCount() function.
    ///
    /// \return Read-only pointer to the array of sound samples
    ///
    /// \see getSampleCount
    ///
    ////////////////////////////////////////////////////////////
    const std::int16_t* getSamples() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of samples stored in the buffer
    ///
    /// The array of samples can be accessed with the getSamples()
    /// function.
    ///
    /// \return Number of samples
    ///
    /// \see getSamples
    ///
    ////////////////////////////////////////////////////////////
    std::uint64_t getSampleCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the sample rate of the sound
    ///
    /// The sample rate is the number of samples played per second.
    /// The higher, the better the quality (for example, 44100
    /// samples/s is CD quality).
    ///
    /// \return Sample rate (number of samples per second)
    ///
    /// \see getChannelCount, getChannelmap, getDuration
    ///
    ////////////////////////////////////////////////////////////
    unsigned int getSampleRate() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of channels used by the sound
    ///
    /// If the sound is mono then the number of channels will
    /// be 1, 2 for stereo, etc.
    ///
    /// \return Number of channels
    ///
    /// \see getSampleRate, getChannelmap, getDuration
    ///
    ////////////////////////////////////////////////////////////
    unsigned int getChannelCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the map of position in sample frame to sound channel
    ///
    /// This is used to map a sample in the sample stream to a
    /// position during spatialisation.
    ///
    /// \return Map of position in sample frame to sound channel
    ///
    /// \see getSampleRate, getChannelCount, getDuration
    ///
    ////////////////////////////////////////////////////////////
    std::vector<SoundChannel> getChannelMap() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the total duration of the sound
    ///
    /// \return Sound duration
    ///
    /// \see getSampleRate, getChannelCount, getChannelmap
    ///
    ////////////////////////////////////////////////////////////
    Time getDuration() const;

    ////////////////////////////////////////////////////////////
    /// \brief Overload of assignment operator
    ///
    /// \param right Instance to assign
    ///
    /// \return Reference to self
    ///
    ////////////////////////////////////////////////////////////
    SoundBuffer& operator=(const SoundBuffer& right);

private:
    friend class Sound;

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Construct from vector of samples
    ///
    ////////////////////////////////////////////////////////////
    explicit SoundBuffer(priv::PassKey<SoundBuffer>&&, std::vector<std::int16_t>&& samples);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Load the sound buffer taking ownership of a vector of audio samples
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<SoundBuffer> loadFromSamplesImpl(
        std::vector<std::int16_t>&&      samples,
        unsigned int                     channelCount,
        unsigned int                     sampleRate,
        const std::vector<SoundChannel>& channelMap);

    ////////////////////////////////////////////////////////////
    /// \brief Initialize the internal state after loading a new sound
    ///
    /// \param file Sound file providing access to the new loaded sound
    ///
    /// \return True on successful initialization, false on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static std::optional<SoundBuffer> initialize(InputSoundFile& file);

    ////////////////////////////////////////////////////////////
    /// \brief Update the internal buffer with the cached audio samples
    ///
    /// \param channelCount Number of channels
    /// \param sampleRate   Sample rate (number of samples per second)
    /// \param channelMap   Map of position in sample frame to sound channel
    ///
    /// \return True on success, false if any error happened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool update(unsigned int channelCount, unsigned int sampleRate, const std::vector<SoundChannel>& channelMap);

    ////////////////////////////////////////////////////////////
    /// \brief Add a sound to the list of sounds that use this buffer
    ///
    /// \param sound Sound instance to attach
    ///
    ////////////////////////////////////////////////////////////
    void attachSound(Sound* sound) const;

    ////////////////////////////////////////////////////////////
    /// \brief Remove a sound from the list of sounds that use this buffer
    ///
    /// \param sound Sound instance to detach
    ///
    ////////////////////////////////////////////////////////////
    void detachSound(Sound* sound) const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    priv::UniquePtr<Impl> m_impl; //!< Implementation details

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDEE(SoundBuffer, Sound);
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::SoundBuffer
/// \ingroup audio
///
/// A sound buffer holds the data of a sound, which is
/// an array of audio samples. A sample is a 16 bits signed integer
/// that defines the amplitude of the sound at a given time.
/// The sound is then reconstituted by playing these samples at
/// a high rate (for example, 44100 samples per second is the
/// standard rate used for playing CDs). In short, audio samples
/// are like texture pixels, and a sf::SoundBuffer is similar to
/// a sf::Texture.
///
/// A sound buffer can be loaded from a file (see loadFromFile()
/// for the complete list of supported formats), from memory, from
/// a custom stream (see sf::InputStream) or directly from an array
/// of samples. It can also be saved back to a file.
///
/// Sound buffers alone are not very useful: they hold the audio data
/// but cannot be played. To do so, you need to use the sf::Sound class,
/// which provides functions to play/pause/stop the sound as well as
/// changing the way it is outputted (volume, pitch, 3D position, ...).
/// This separation allows more flexibility and better performances:
/// indeed a sf::SoundBuffer is a heavy resource, and any operation on it
/// is slow (often too slow for real-time applications). On the other
/// side, a sf::Sound is a lightweight object, which can use the audio data
/// of a sound buffer and change the way it is played without actually
/// modifying that data. Note that it is also possible to bind
/// several sf::Sound instances to the same sf::SoundBuffer.
///
/// It is important to note that the sf::Sound instance doesn't
/// copy the buffer that it uses, it only keeps a reference to it.
/// Thus, a sf::SoundBuffer must not be destructed while it is
/// used by a sf::Sound (i.e. never write a function that
/// uses a local sf::SoundBuffer instance for loading a sound).
///
/// Usage example:
/// \code
/// // Load a new sound buffer from a file
/// const auto buffer = sf::SoundBuffer::loadFromFile("sound.wav").value();
///
/// // Create a sound source bound to the buffer
/// sf::Sound sound1(buffer);
///
/// // Play the sound
/// sound1.play();
///
/// // Create another sound source bound to the same buffer
/// sf::Sound sound2(buffer);
///
/// // Play it with a higher pitch -- the first sound remains unchanged
/// sound2.setPitch(2);
/// sound2.play();
/// \endcode
///
/// \see sf::Sound, sf::SoundBufferRecorder
///
////////////////////////////////////////////////////////////
