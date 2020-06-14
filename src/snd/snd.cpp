/*
@file

    snd.cpp

@purpose

    Load/manage and play music/.wav files from memory
*/

#include "snd.h"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
#include <SFML/Audio.hpp>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#ifdef PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif
#endif

// all our sounds
#include "ding.h"
#include "pause.h"
#include "eat.h"

// from imgui_draw.cpp
extern unsigned int stb_decompress_length(const unsigned char *input);
extern unsigned int stb_decompress(unsigned char *output, const unsigned char *input, unsigned int length);

/*
@brief

    Constructor
*/
retrogames::snd_t::snd_t() : initialized(false) {}

/*
@brief

    Tries to initialize all our sounds
*/
bool retrogames::snd_t::initialize(void)
{
    auto decompress = [](const uint32_t size, const uint32_t* data) -> std::pair<uint32_t, uint8_t*>
    {
        const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)data);
        unsigned char* buf_decompressed_data = new unsigned char[buf_decompressed_size];
        stb_decompress(buf_decompressed_data, (const unsigned char*)data, (unsigned int)size);

        return std::make_pair<uint32_t, uint8_t*>(static_cast<uint32_t>(buf_decompressed_size), static_cast<uint8_t*>(buf_decompressed_data));
    };

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
    auto add_sound = [this, &decompress](sounds_e sound, const uint32_t size, const uint32_t* data) -> bool
    {
        auto sound_raw = (sounds_raw.at(static_cast<std::size_t>(sound)) = decompress(size, data));
        auto sound_ptr = (sound_buffers.at(static_cast<std::size_t>(sound)) = new sf::SoundBuffer());

        if (!sound_ptr->loadFromMemory(static_cast<const void*>(sound_raw.second), static_cast<std::size_t>(sound_raw.first)))
        {
            // delete all our sounds that were added until now
            for (int32_t i = static_cast<int32_t>(sound); i >= 0; i--)
            {
                delete sounds_raw.at(static_cast<std::size_t>(i)).second;
                delete sound_buffers.at(static_cast<std::size_t>(i));

                if (i != static_cast<int32_t>(sound)) delete sounds.at(static_cast<std::size_t>(i));
            }

            // failed to load sound
            return false;
        }

        sounds.at(static_cast<std::size_t>(sound)) = new sf::Sound(*sound_ptr);

        return true;
    };
#else

#ifndef PLATFORM_EMSCRIPTEN
    // init SDL2 audio
    SDL_Init(SDL_INIT_AUDIO);
#endif

#ifdef PLATFORM_EMSCRIPTEN
    int const frequency = EM_ASM_INT({
        var AudioContext = window.AudioContext || window.webkitAudioContext;
        var ctx = new AudioContext();
        var sr = ctx.sampleRate;
        ctx.close();
        return sr;
    });
#else
    int const frequency = 44100;
#endif

    // open 44.1KHz, signed 16bit, system byte order,
    // stereo audio, using 4096 byte chunks
    if (Mix_OpenAudio(frequency, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) != 0) return false;

    auto add_sound = [this, &decompress](sounds_e sound, const uint32_t size, const uint32_t* data) -> bool
    {
        auto& sound_raw = (sounds_raw.at(static_cast<std::size_t>(sound)) = decompress(size, data));
        auto sound_buffer = (sound_buffers.at(static_cast<std::size_t>(sound)) = SDL_RWFromMem(reinterpret_cast<void*>(sound_raw.second), static_cast<int32_t>(sound_raw.first)));

        if (sound_buffer == nullptr)
        {
            // delete all our sounds that were added until now
            for (int32_t i = static_cast<int32_t>(sound); i >= 0; i--)
            {
                delete sounds_raw.at(static_cast<std::size_t>(i)).second;

                if (i != static_cast<int32_t>(sound)) SDL_RWclose(sound_buffers.at(static_cast<std::size_t>(i)));
            }

            return false;
        }

        if ((sounds.at(static_cast<std::size_t>(sound)) = Mix_LoadWAV_RW(sound_buffer, 0)) == nullptr)
        {
            // delete all our sounds that were added until now
            for (int32_t i = static_cast<int32_t>(sound); i >= 0; i--)
            {
                delete sounds_raw.at(static_cast<std::size_t>(i)).second;

                SDL_RWclose(sound_buffers.at(static_cast<std::size_t>(i)));

                if (i != static_cast<int32_t>(sound)) Mix_FreeChunk(sounds.at(static_cast<std::size_t>(i)));
            }

            return false;
        }

        return true;
    };
#endif

    // add our sounds - WARNING! ADD SOUNDS IN THE RIGHT ORDER (0-SOUND_SIZE-1)!!!!
    // otherwise memory leaks and/or crashes WILL occur!
    if (!add_sound(sounds_e::SOUND_DING, dingwav_compressed_size, dingwav_compressed_data)) return false;
    if (!add_sound(sounds_e::SOUND_PAUSE, pause_compressed_size, pause_compressed_data)) return false;
    if (!add_sound(sounds_e::SOUND_EAT, eat_compressed_size, eat_compressed_data)) return false;

    // all done!
    initialized = true;

    // all our sounds loaded successfully
    return true;
}

/*
@brief

    Destructor
*/
retrogames::snd_t::~snd_t()
{
    if (!initialized) return;

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
    // clean up our sounds
    for (std::size_t i = 0; i < sound_buffers.size(); i++)
    {
        delete sounds.at(i);
        delete sound_buffers.at(i);
        delete[] sounds_raw.at(i).second;
    }
#else
    // clean up our sounds
    for (std::size_t i = 0; i < sound_buffers.size(); i++)
    {
        Mix_FreeChunk(sounds.at(i));

        SDL_RWclose(sound_buffers.at(i));

        delete[] sounds_raw.at(i).second;
    }

    // quit SDL
    SDL_Quit();
#endif
}

/*
@brief

    Plays a @sound with the specified @volume
*/
void retrogames::snd_t::play_sound(sounds_e sound, float volume/* = 100.f*/)
{
    auto snd = sounds.at(static_cast<std::size_t>(sound));

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
    if (snd->getVolume() != volume) snd->setVolume(volume);

    snd->play();
#else
    auto vol = static_cast<int32_t>((volume / 100.f) * static_cast<float>(MIX_MAX_VOLUME));

    if (Mix_VolumeChunk(snd, -1) != vol) Mix_VolumeChunk(snd, vol);

    Mix_PlayChannelTimed(-1, snd, 0, -1);
#endif
}