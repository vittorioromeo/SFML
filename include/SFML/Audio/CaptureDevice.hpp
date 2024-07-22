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

#include <SFML/System/LifetimeDependant.hpp>
#include <SFML/System/LifetimeDependee.hpp>

#include <SFML/Base/Optional.hpp>
#include <SFML/Base/UniquePtr.hpp>

#include <vector>

#include <cstddef>
#include <cstdint>


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace sf
{
class AudioContext;
class CaptureDeviceHandle;
class SoundRecorder;
} // namespace sf


namespace sf
{
////////////////////////////////////////////////////////////
class CaptureDevice
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Create the default capture device from `audioContext`
    ///
    /// \return Capture device on success, `sf::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<CaptureDevice> createDefault(AudioContext& audioContext);

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    explicit CaptureDevice(AudioContext& audioContext, const CaptureDeviceHandle& deviceHandle);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~CaptureDevice();

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    CaptureDevice(CaptureDevice&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    CaptureDevice& operator=(CaptureDevice&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Get the device handle
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const CaptureDeviceHandle& getDeviceHandle() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the sample rate of the capture device
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setSampleRate(unsigned int sampleRate);

    ////////////////////////////////////////////////////////////
    /// \brief Get the sample rate
    ///
    /// The sample rate defines the number of audio samples
    /// captured per second. The higher, the better the quality
    /// (for example, 44100 samples/sec is CD quality).
    ///
    /// \return Sample rate, in samples per second
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getSampleRate() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the channel count of the audio capture device
    ///
    /// This method allows you to specify the number of channels
    /// used for recording. Currently only 16-bit mono and
    /// 16-bit stereo are supported.
    ///
    /// \param channelCount Number of channels. Currently only
    ///                     mono (1) and stereo (2) are supported.
    ///
    /// \see getChannelCount
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool setChannelCount(unsigned int channelCount);

    ////////////////////////////////////////////////////////////
    /// \brief Get the number of channels used by this recorder
    ///
    /// Currently only mono and stereo are supported, so the
    /// value is either 1 (for mono) or 2 (for stereo).
    ///
    /// \return Number of channels
    ///
    /// \see setChannelCount
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] unsigned int getChannelCount() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the map of position in sample frame to sound channel
    ///
    /// This is used to map a sample in the sample stream to a
    /// position during spatialisation.
    ///
    /// \return Map of position in sample frame to sound channel
    ///
    ////////////////////////////////////////////////////////////
    const std::vector<SoundChannel>& getChannelMap() const;

private:
    friend SoundRecorder;

    ////////////////////////////////////////////////////////////
    /// \brief Returns `true` if the device is initialized
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDeviceInitialized() const;

    ////////////////////////////////////////////////////////////
    /// \brief Returns `true` if the device is started
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isDeviceStarted() const;

    ////////////////////////////////////////////////////////////
    /// \brief Try to start the device, returns `true` on success
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool startDevice() const;

    ////////////////////////////////////////////////////////////
    /// \brief Try to stop the device, returns `true` on success
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool stopDevice() const;

    ////////////////////////////////////////////////////////////
    /// \brief Set the "process samples" callback used by miniaudio
    ///
    ////////////////////////////////////////////////////////////
    using ProcessSamplesFunc = bool (*)(void* userData, const std::int16_t* samples, std::size_t sampleCount);
    void setProcessSamplesFunc(SoundRecorder* soundRecorder, ProcessSamplesFunc processSamplesFunc);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::UniquePtr<Impl> m_impl; //!< Implementation details

    ////////////////////////////////////////////////////////////
    // Lifetime tracking
    ////////////////////////////////////////////////////////////
    SFML_DEFINE_LIFETIME_DEPENDANT(AudioContext);
    SFML_DEFINE_LIFETIME_DEPENDEE(CaptureDevice, SoundRecorder);
};

} // namespace sf
