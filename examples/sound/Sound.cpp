////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/AudioContext.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/PlaybackDevice.hpp>
#include <SFML/Audio/PlaybackDeviceHandle.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <SFML/System/Path.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>

#include <iostream>


////////////////////////////////////////////////////////////
/// Play a sound
///
////////////////////////////////////////////////////////////
void playSound(sf::PlaybackDevice& playbackDevice)
{
    // Load a sound buffer from a wav file
    const auto buffer = sf::SoundBuffer::loadFromFile("resources/killdeer.wav").value();

    // Display sound information
    std::cout << "killdeer.wav:" << '\n'
              << " " << buffer.getDuration().asSeconds() << " seconds" << '\n'
              << " " << buffer.getSampleRate() << " samples / sec" << '\n'
              << " " << buffer.getChannelCount() << " channels" << '\n';

    // Create a sound instance and play it
    sf::Sound sound(buffer);
    sound.play(playbackDevice);

    // Loop while the sound is playing
    while (sound.getStatus() == sf::Sound::Status::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        std::cout << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        " << std::flush;
    }

    std::cout << '\n' << std::endl;
}


////////////////////////////////////////////////////////////
/// Play a music
///
////////////////////////////////////////////////////////////
void playMusic(sf::PlaybackDevice& playbackDevice, const sf::Path& filename)
{
    // Load an ogg music file
    auto music = sf::Music::openFromFile("resources" / filename).value();

    // Display music information
    std::cout << filename << ":" << '\n'
              << " " << music.getDuration().asSeconds() << " seconds" << '\n'
              << " " << music.getSampleRate() << " samples / sec" << '\n'
              << " " << music.getChannelCount() << " channels" << '\n';

    // Play it
    music.play(playbackDevice);

    // Loop while the music is playing
    while (music.getStatus() == sf::Music::Status::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        std::cout << "\rPlaying... " << music.getPlayingOffset().asSeconds() << " sec        " << std::flush;
    }

    std::cout << '\n' << std::endl;
}


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create an audio context and get the default playback device
    auto audioContext   = sf::AudioContext::create().value();
    auto playbackDevice = sf::PlaybackDevice::createDefault(audioContext).value();

    // Play a sound
    playSound(playbackDevice);

    // Play music from an ogg file
    playMusic(playbackDevice, "doodle_pop.ogg");

    // Play music from a flac file
    playMusic(playbackDevice, "ding.flac");

    // Play music from a mp3 file
    playMusic(playbackDevice, "ding.mp3");

    // Wait until the user presses 'enter' key
    std::cout << "Press enter to exit..." << std::endl;
    std::cin.ignore(10000, '\n');
}
