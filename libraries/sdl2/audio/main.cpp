#include <SDL2/SDL.h>
#include <cmath>

const float PI = 3.14159;
const float TAU = PI * 2.0f;

struct AudioCallbackData
{
    float volume;
    float frequency;
    Sint64 pos;
};

void audioCallack(void* userdata, Uint8* byte_stream, int bytes_requested)
{
    SDL_memset(byte_stream, 0, bytes_requested); // set the buffer to silence

    Uint16* buf              = (Uint16*)byte_stream;
    int samples_requested   = bytes_requested / sizeof(*buf);

    AudioCallbackData* m    = (AudioCallbackData*)userdata;

    for( int i = 0; i < samples_requested; ++i )
    {
        buf[i] = std::sin(TAU * m->pos * m->frequency) * m->volume * (float)SDL_MAX_SINT16;

        m->pos++;
    }

    SDL_Log("%d samples written", samples_requested);
}

int main(int argc, char* argv[])
{
    if(SDL_Init(SDL_INIT_AUDIO))
    {
        SDL_Log("ERROR: %s", SDL_GetError());
        return -1;
    }

    int num_audio_drivers = SDL_GetNumAudioDrivers();
    SDL_Log("Found %d audio drivers", num_audio_drivers);

    {
        const char* current_driver = SDL_GetCurrentAudioDriver();

        for(int i = 0; i < num_audio_drivers; ++i)
        {
            const char* driver = SDL_GetAudioDriver(i);

            if( strcmp(current_driver, driver) == 0)
                SDL_Log("%d) %s (selected)", i, driver);
            else
                SDL_Log("%d) %s", i, driver);
        }
    }

    int num_audio_devices = SDL_GetNumAudioDevices(0);
    SDL_Log("Found %d audio devices\n", num_audio_devices); 

    for(int i = 0; i < num_audio_devices; ++i)
    {
        const char* name = SDL_GetAudioDeviceName(i, 0);
        SDL_Log("%d) %s\n", i, name);
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);
    SDL_zero(have); 

    // https://wiki.libsdl.org/SDL_AudioSpec

    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 4096;

    AudioCallbackData callbackData;

    want.callback = audioCallack;
    want.userdata = &callbackData;

    // https://wiki.libsdl.org/SDL_OpenAudioDevice
    //
    // pass in flags here if you are willing to accept an audio device that doesn't exactly match the spec you asked for
    // or 0 if you will only accept exactly the audio spec you asked for, which will cause SDL to do any necessary conversion for you

    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    SDL_Log("Desired : freq: %d, format: (size: %dbits, float: %d, signed: %d, little-endian: %d,), channels: %d, samples: %d",
        want.freq, SDL_AUDIO_BITSIZE(want.format), (bool)SDL_AUDIO_ISFLOAT(want.format), (bool)SDL_AUDIO_ISSIGNED(want.format), (bool)SDL_AUDIO_ISLITTLEENDIAN(want.format), want.channels, want.samples);

    SDL_Log("Obtained: freq: %d, format: (size: %dbits, float: %d, signed: %d, little-endian: %d,), channels: %d, samples: %d",
        have.freq, SDL_AUDIO_BITSIZE(have.format), (bool)SDL_AUDIO_ISFLOAT(have.format), (bool)SDL_AUDIO_ISSIGNED(have.format), (bool)SDL_AUDIO_ISLITTLEENDIAN(have.format), have.channels, have.samples);

    if(!device)
    {
        SDL_Log("ERROR: failed to open audio device");
        return -1;
    };

    const float frequncey = 500.0f;

    callbackData.volume = 0.5f;
    callbackData.frequency = frequncey / (float)have.freq;
    callbackData.pos = 0;

    SDL_PauseAudioDevice(device, 0); // unpause the audio device

    SDL_Delay(2000);

    SDL_CloseAudioDevice(device);
    SDL_Quit();

    return 0;
}