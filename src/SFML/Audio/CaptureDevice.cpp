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
#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/CaptureDevice.hpp>
#include <SFML/Audio/CaptureDeviceHandle.hpp>
#include <SFML/Audio/SoundChannel.hpp>

#include <SFML/System/Err.hpp>

#include <miniaudio.h>

#include <vector>

#include <cassert>
#include <cstring>


namespace sf
{
////////////////////////////////////////////////////////////
struct CaptureDevice::Impl
{
    static void maDeviceDataCallback(ma_device* device, void*, const void* input, ma_uint32 frameCount)
    {
        auto& impl = *static_cast<Impl*>(device->pUserData);

        // Copy the new samples into our temporary buffer
        impl.samples.resize(frameCount * impl.channelCount);
        std::memcpy(impl.samples.data(), input, frameCount * impl.channelCount * sizeof(std::int16_t));

        // Notify the derived class of the availability of new samples
        assert(impl.processSamplesFunc != nullptr && "processSamplesFunc callback not registered in capture device");
        assert(impl.soundRecorder != nullptr && "processSamplesFunc callback user data is null");
        if (impl.processSamplesFunc(impl.soundRecorder, impl.samples.data(), impl.samples.size()))
            return;

        // If the derived class wants to stop, stop the capture
        if (const auto result = ma_device_stop(device); result != MA_SUCCESS)
            priv::err() << "Failed to stop audio capture device: " << ma_result_description(result) << priv::errEndl;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard]] explicit Impl(AudioContext& theAudioContext,
                                // NOLINTNEXTLINE(modernize-pass-by-value)
                                const CaptureDeviceHandle& theDeviceHandle) :
    audioContext(&theAudioContext),
    captureDeviceHandle(theDeviceHandle)
    {
    }

    ~Impl()
    {
        deinitialize();
    }

    void deinitialize()
    {
        ma_device_uninit(&maDevice);
    }

    [[nodiscard]] bool initialize()
    {
        ma_device_config captureDeviceConfig = ma_device_config_init(ma_device_type_capture);
        captureDeviceConfig.dataCallback     = maDeviceDataCallback;
        captureDeviceConfig.pUserData        = this;
        captureDeviceConfig.capture
            .pDeviceID = &static_cast<const ma_device_info*>(captureDeviceHandle.getMADeviceInfo())->id;
        captureDeviceConfig.capture.channels = channelCount;
        captureDeviceConfig.capture.format   = ma_format_s16;
        captureDeviceConfig.sampleRate       = sampleRate;

        if (const auto result = ma_device_init(&*static_cast<ma_context*>(audioContext->getMAContext()),
                                               &captureDeviceConfig,
                                               &maDevice);
            result != MA_SUCCESS)
        {
            priv::err() << "Failed to initialize the audio device: " << ma_result_description(result) << priv::errEndl;
            return false;
        }

        return true;
    }

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    AudioContext*             audioContext;                   //!< Audio context
    CaptureDeviceHandle       captureDeviceHandle;            //!< Capture device handle
    ma_uint32                 channelCount{1u};               //!< Number of recording channels
    ma_uint32                 sampleRate{44100u};             //!< Sample rate
    std::vector<std::int16_t> samples;                        //!< Buffer to store captured samples
    std::vector<SoundChannel> channelMap{SoundChannel::Mono}; //!< The map of position in sample frame to sound channel

    SoundRecorder*     soundRecorder{nullptr}; //!< Used in the miniaudio device callback
    ProcessSamplesFunc processSamplesFunc{};   //!< Used in the miniaudio device callback

    ma_device maDevice; //!< miniaudio capture device (one per hardware device)
};


////////////////////////////////////////////////////////////
std::optional<CaptureDevice> CaptureDevice::createDefault(AudioContext& audioContext)
{
    std::optional defaultCaptureDeviceHandle = audioContext.getDefaultCaptureDeviceHandle();

    if (!defaultCaptureDeviceHandle.has_value())
        return std::nullopt;

    return std::make_optional<CaptureDevice>(audioContext, *defaultCaptureDeviceHandle);
}


////////////////////////////////////////////////////////////
CaptureDevice::CaptureDevice(AudioContext& audioContext, const CaptureDeviceHandle& playbackDeviceHandle) :
m_impl(priv::makeUnique<Impl>(audioContext, playbackDeviceHandle))
{
    if (!m_impl->initialize())
        priv::err() << "Failed to initialize the capture device" << priv::errEndl;

    SFML_UPDATE_LIFETIME_DEPENDANT(AudioContext, CaptureDevice, m_impl->audioContext);
}


////////////////////////////////////////////////////////////
CaptureDevice::~CaptureDevice()
{
    if (m_impl == nullptr) // Could be moved
        return;

    assert(!ma_device_is_started(&m_impl->maDevice) &&
           "The miniaudio capture device must be stopped before destroying the capture device");
}


////////////////////////////////////////////////////////////
CaptureDevice::CaptureDevice(CaptureDevice&&) noexcept = default;


////////////////////////////////////////////////////////////
CaptureDevice& CaptureDevice::operator=(CaptureDevice&&) noexcept = default;


////////////////////////////////////////////////////////////
[[nodiscard]] const CaptureDeviceHandle& CaptureDevice::getDeviceHandle() const
{
    return m_impl->captureDeviceHandle;
}


////////////////////////////////////////////////////////////
bool CaptureDevice::setSampleRate(unsigned int sampleRate)
{
    // Store the sample rate and re-initialize if necessary
    if (m_impl->sampleRate != sampleRate)
    {
        m_impl->sampleRate = sampleRate;

        m_impl->deinitialize();
        if (!m_impl->initialize())
        {
            priv::err() << "Failed to set audio capture device sample rate to " << sampleRate << priv::errEndl;
            return false;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////
unsigned int CaptureDevice::getSampleRate() const
{
    return m_impl->sampleRate;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool CaptureDevice::isDeviceInitialized() const
{
    return ma_device_get_state(&m_impl->maDevice) != ma_device_state_uninitialized;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool CaptureDevice::isDeviceStarted() const
{
    return ma_device_is_started(&m_impl->maDevice);
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool CaptureDevice::startDevice() const
{
    assert(isDeviceInitialized() && "Attempted to start an uninitialized audio capture device");
    assert(!isDeviceStarted() && "Attempted to start an already started audio capture device");

    if (const auto result = ma_device_start(&m_impl->maDevice); result != MA_SUCCESS)
    {
        priv::err() << "Failed to start audio capture device: " << ma_result_description(result) << priv::errEndl;
        return false;
    }

    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool CaptureDevice::stopDevice() const
{
    assert(isDeviceInitialized() && "Attempted to stop an uninitialized audio capture device");
    assert(isDeviceStarted() && "Attempted to stop an already stopped audio capture device");

    if (const auto result = ma_device_stop(&m_impl->maDevice); result != MA_SUCCESS)
    {
        priv::err() << "Failed to stop audio capture device: " << ma_result_description(result) << priv::errEndl;
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////
bool CaptureDevice::setChannelCount(unsigned int channelCount)
{
    // We only bother supporting mono/stereo recording for now
    if (channelCount < 1 || channelCount > 2)
    {
        priv::err() << "Unsupported channel count: " << channelCount
                    << " Currently only mono (1) and stereo (2) recording is supported." << priv::errEndl;

        return false;
    }

    // Store the channel count and re-initialize if necessary
    if (m_impl->channelCount == channelCount)
        return true;

    m_impl->channelCount = channelCount;

    m_impl->deinitialize();
    if (!m_impl->initialize())
    {
        priv::err() << "Failed to set audio capture device channel count to " << channelCount << priv::errEndl;
        return false;
    }

    // We only bother supporting mono/stereo recording for now
    if (channelCount == 1)
    {
        m_impl->channelMap = {SoundChannel::Mono};
    }
    else if (channelCount == 2)
    {
        m_impl->channelMap = {SoundChannel::FrontLeft, SoundChannel::FrontRight};
    }

    return true;
}


////////////////////////////////////////////////////////////
unsigned int CaptureDevice::getChannelCount() const
{
    return m_impl->channelCount;
}


////////////////////////////////////////////////////////////
const std::vector<SoundChannel>& CaptureDevice::getChannelMap() const
{
    return m_impl->channelMap;
}


////////////////////////////////////////////////////////////
void CaptureDevice::setProcessSamplesFunc(SoundRecorder* soundRecorder, ProcessSamplesFunc processSamplesFunc)
{
    m_impl->soundRecorder      = soundRecorder;
    m_impl->processSamplesFunc = processSamplesFunc;
}

} // namespace sf