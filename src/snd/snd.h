/*
@file

    snd.h

@purpose

    Load/manage and play music/.wav files from memory
*/

#pragma once

#include <array>
#include <stdint.h>

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
namespace sf
{

    class SoundBuffer;
    class Sound;

}
#else
struct SDL_RWops;
struct Mix_Chunk;
#endif

namespace retrogames
{

    class snd_t final
    {

    protected:



    public:

        enum class sounds_e
        {

            SOUND_DING,
            SOUND_PAUSE,
            SOUND_EAT,

            SOUND_SIZE

        };

    private:

        std::array<std::pair<uint32_t, uint8_t*>, static_cast<std::size_t>(sounds_e::SOUND_SIZE)> sounds_raw;

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
        std::array<sf::SoundBuffer*, static_cast<std::size_t>(sounds_e::SOUND_SIZE)> sound_buffers;
        std::array<sf::Sound*, static_cast<std::size_t>(sounds_e::SOUND_SIZE)> sounds;
#else
        std::array<SDL_RWops*, static_cast<std::size_t>(sounds_e::SOUND_SIZE)> sound_buffers;
        std::array<Mix_Chunk*, static_cast<std::size_t>(sounds_e::SOUND_SIZE)> sounds;
#endif

        bool initialized;

    public:

        /*
        @brief

            Constructor
        */
        snd_t();

        /*
        @brief

            Destructor
        */
        ~snd_t();

        /*
        @brief

            Tries to initialize all our sounds
        */
        bool initialize(void);

        /*
        @brief

            Plays a @sound with the specified @volume
        */
        void play_sound(sounds_e sound, float volume = 100.f);

    };

    extern snd_t* snd;

}