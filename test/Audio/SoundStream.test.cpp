#include "SFML/Audio/SoundStream.hpp"

#include "SFML/Audio/AudioContext.hpp"
#include "SFML/Audio/PlaybackDevice.hpp"

#include "SFML/System/Time.hpp"

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <CommonTraits.hpp>
#include <SystemUtil.hpp>

namespace
{
class TestSoundStream : public sf::SoundStream
{
public:
    explicit TestSoundStream() = default;

protected:
    [[nodiscard]] bool onGetData(Chunk& /* data */) override
    {
        return true;
    }

    void onSeek(sf::Time /* timeOffset */) override
    {
    }
};
} // namespace

TEST_CASE("[Audio] sf::SoundStream" * doctest::skip(skipAudioDeviceTests))
{
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::SoundStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundStream));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundStream));
        STATIC_CHECK(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::SoundStream));
    }

    SECTION("Chunk")
    {
        const sf::SoundStream::Chunk chunk;
        CHECK(chunk.samples == nullptr);
        CHECK(chunk.sampleCount == 0);
    }

    SECTION("Construction")
    {
        const TestSoundStream testSoundStream;
        CHECK(testSoundStream.getChannelCount() == 0);
        CHECK(testSoundStream.getSampleRate() == 0);
        CHECK(testSoundStream.getStatus() == sf::SoundStream::Status::Stopped);
        CHECK(testSoundStream.getPlayingOffset() == sf::Time::Zero);
        CHECK(!testSoundStream.isLooping());
    }

    SECTION("Set/get playing offset")
    {
        TestSoundStream testSoundStream;
        testSoundStream.setPlayingOffset(sf::milliseconds(100));
        CHECK(testSoundStream.getPlayingOffset() == sf::milliseconds(0));
    }

    SECTION("Set/get loop")
    {
        TestSoundStream testSoundStream;
        testSoundStream.setLooping(true);
        CHECK(testSoundStream.isLooping());
    }
}
