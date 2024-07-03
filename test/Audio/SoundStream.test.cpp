#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/SoundStream.hpp>

#include <SFML/System/Time.hpp>

#include <Doctest.hpp>

#include <AudioUtil.hpp>
#include <SystemUtil.hpp>

#include <type_traits>

namespace
{
class TestSoundStream : public sf::SoundStream
{
public:
    explicit TestSoundStream(sf::PlaybackDevice& playbackDevice) : sf::SoundStream(playbackDevice)
    {
    }

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
    sf::PlaybackDevice playbackDevice;

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_constructible_v<sf::SoundStream>);
        STATIC_CHECK(!std::is_copy_constructible_v<sf::SoundStream>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::SoundStream>);
        STATIC_CHECK(!std::is_nothrow_move_constructible_v<sf::SoundStream>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::SoundStream>);
        STATIC_CHECK(std::has_virtual_destructor_v<sf::SoundStream>);
    }

    SECTION("Chunk")
    {
        const sf::SoundStream::Chunk chunk;
        CHECK(chunk.samples == nullptr);
        CHECK(chunk.sampleCount == 0);
    }

    SECTION("Construction")
    {
        const TestSoundStream testSoundStream(playbackDevice);
        CHECK(testSoundStream.getChannelCount() == 0);
        CHECK(testSoundStream.getSampleRate() == 0);
        CHECK(testSoundStream.getStatus() == sf::SoundStream::Status::Stopped);
        CHECK(testSoundStream.getPlayingOffset() == sf::Time::Zero);
        CHECK(!testSoundStream.getLoop());
    }

    SECTION("Set/get playing offset")
    {
        TestSoundStream testSoundStream(playbackDevice);
        testSoundStream.setPlayingOffset(sf::milliseconds(100));
        CHECK(testSoundStream.getPlayingOffset() == sf::milliseconds(0));
    }

    SECTION("Set/get loop")
    {
        TestSoundStream testSoundStream(playbackDevice);
        testSoundStream.setLoop(true);
        CHECK(testSoundStream.getLoop());
    }
}
