
#include <stdint.h>
#include <iostream>
#include <queue>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


// TODO: Consider OpenAL or PulseAudio instead, to avoid the latency
// I'm seeing with SDL audio.
//
// Don't forget to remove SDL_INIT_AUDIO from SDL_Init!
//


const uint32_t WINDOW_WIDTH = 1024;
const uint32_t WINDOW_HEIGHT = 768;



enum class Note
{
    C,
    Cs,
    D,
    Eb,
    E,
    F,
    Fs,
    G,
    Gs,
    A,
    Bb,
    B
};

double noteFrequncy(Note note, uint8_t octave)
{
    double base;
    switch (note)
    {
        case Note::C:
            base = 16.35;
            break;
        case Note::Cs:
            base = 17.32;
            break;
        case Note::D:
            base = 18.35;
            break;
        case Note::Eb:
            base = 19.45;
            break;
        case Note::E:
            base = 20.60;
            break;
        case Note::F:
            base = 21.83;
            break;
        case Note::Fs:
            base = 21.83;
            break;
        case Note::G:
            base = 24.50;
            break;
        case Note::Gs:
            base = 25.96;
            break;
        case Note::A:
            base = 27.50;
            break;
        case Note::Bb:
            base = 29.14;
            break;
        case Note::B:
            base = 30.87;
            break;
        default:
            base = 100.00;
            break;
    }
    return base * std::pow(2.0, octave);
}



struct Voice
{
    std::queue<int16_t> samples;
    bool playing;
};


#define NUM_VOICES   4
struct PlaybackUserData
{
    Voice voices[NUM_VOICES];
};



const uint32_t fs = 44100;

void makeSamples(std::queue<int16_t>& rSampleData, double f) {
    int numSamples = fs / 10;
    for (int i = 0; i < numSamples; i++)
    {
        double t = static_cast<double>(i) / static_cast<double>(fs);
        // sampleData[i] = static_cast<int16_t>(0x7fff * std::sin(2 * M_PI * 440.0 * t));

        const double tau { 2 * M_PI };

        const double A = 1.0;
        const double C = f * tau;
        const double M = C / 4;
        // const double I = t * 5;
        const double I = 0;


        auto square = [](double phase) -> double {
            return std::sin(phase) < 0 ? -1.0 : 1.0;
        };

        // auto sawtooth = [](double phase) -> double { };

        // auto triangle = [](double phase) -> double {
        //     phase = std::fmod(phase, 2.0 * M_PI);
        //     if (phase < M_PI / 2.0) {

        //     }
        //     else if (phase < M_PI) {

        //     }
        //     else if (phase < 1.5 * M_PI) {

        //     }
        //     else {

        //     }
        // };

        // double value = A * std::sin(C * t + I * std::sin(M * t));
        double value = A * square(C * t + I * square(M * t));

        rSampleData.push(static_cast<int16_t>(0x7fff * value));
    }
}




// TODO: Use a lambda
void audio_callback(void* pUserdata, uint8_t* pStream, int length)
{
    PlaybackUserData* pPlaybackData = reinterpret_cast<PlaybackUserData*>(pUserdata);
    for (int i = 0; i < length / sizeof(int16_t); i++)
    {
        int16_t sample = 0;
        for (int j = 0; j < NUM_VOICES; j++)
        {
            Voice& rVoice = pPlaybackData->voices[j];
            if (rVoice.playing && ! rVoice.samples.empty())
            {
                sample += rVoice.samples.front() / 4;
                rVoice.samples.pop();
            }
        }
        *(reinterpret_cast<int16_t*>(pStream) + i) = sample;
    }
}







int main()
{

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cerr << "Failed to init SDL" << std::endl;
        return 1;
    }

    if (TTF_Init() != 0)
    {
        std::cerr << "Failed to init SDL TTF" << std::endl;
        return 1;
    }

    SDL_Window* pWindow = SDL_CreateWindow(
        "softsynth",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (pWindow == nullptr)
    {
        std::cerr << "Failed to create SDL window" << std::endl;
        return 1;
    }

    SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (pRenderer == nullptr)
    {
        std::cerr << "Failed to create SDL renderer" << std::endl;
        return 1;
    }

    SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_BLEND);

    const uint32_t numSamples = fs * 3;
    // int16_t sampleData[numSamples];


    PlaybackUserData userdata;
    userdata.voices[0].playing = false;
    userdata.voices[1].playing = false;
    userdata.voices[2].playing = false;
    userdata.voices[3].playing = false;



    makeSamples(userdata.voices[0].samples, noteFrequncy(Note::C, 4));
    makeSamples(userdata.voices[1].samples, noteFrequncy(Note::D, 4));
    makeSamples(userdata.voices[2].samples, noteFrequncy(Note::E, 4));
    makeSamples(userdata.voices[3].samples, noteFrequncy(Note::F, 4));



    // https://wiki.libsdl.org/SDL_AudioSpec
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID device;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = fs;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audio_callback;
    want.userdata = &userdata;
    device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (device == 0)
    {
        std::cerr << "Failed to init SDL audio" << std::endl;
        return 1;
    }

    // There is a super annoying delay which may make a keyboard impossible
    // to implement using SDL's audio. Apparently OpenAL can be better for
    // this.
    // https://www.gamedev.net/forums/topic/228019-sound-programming---audio-latency--unresponsiveness-in-sdl/

    // SDL_QueueAudio(device, sampleData, numSamples);
    SDL_PauseAudioDevice(device, 0);
    // SDL_Delay(1000);
    // SDL_PauseAudioDevice(device, 1);
    // SDL_Delay(3000);
    // SDL_PauseAudioDevice(device, 0);


    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 24);
    if (font == nullptr)
    {
        std::cerr << "Failed to open font" << std::endl;
        return 1;
    }

    // https://www.libsdl.org/projects/SDL_ttf/docs/SDL_ttf_42.html

    // SDL_Color back_color = {0x00, 0x00, 0x00, 0x00};
    SDL_Color fore_color = {0xff, 0xff, 0xff, 0xff};
    // SDL_Surface* msg = TTF_RenderText_Shaded(font, "Hello World!", fore_color, back_color);
    SDL_Surface* msg = TTF_RenderText_Solid(font, "Hello World!", fore_color);
    SDL_Texture* msgTexture = SDL_CreateTextureFromSurface(pRenderer, msg);

    SDL_Rect msgRect;
    msgRect.x = 32;
    msgRect.y = 32;
    msgRect.w = msg->w;
    msgRect.h = msg->h;
    SDL_FreeSurface(msg);






    bool paused = false;

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                {
                    running = false;
                    break;
                }

                case SDL_KEYDOWN:
                {
                    switch (event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                        case SDLK_q:
                            running = false;
                            break;

                        // case SDLK_p:
                        //     // paused = ! paused;
                        //     // SDL_PauseAudioDevice(device, paused);
                        //     userdata.playing = ! userdata.playing;
                        //     break;
                    }
                    break;
                }
            }
        }


        static bool lastKeyStateP = false;
        const uint8_t *pSdlKeyboardState = SDL_GetKeyboardState(NULL);
        userdata.voices[0].playing = pSdlKeyboardState[SDL_SCANCODE_U];
        userdata.voices[1].playing = pSdlKeyboardState[SDL_SCANCODE_I];
        userdata.voices[2].playing = pSdlKeyboardState[SDL_SCANCODE_O];
        userdata.voices[3].playing = pSdlKeyboardState[SDL_SCANCODE_P];


        // This is not sufficient. It sounds like a helicopter.
        // Presumably the voice is running out of data.
        // Or the new data has some kind of crossover-like distortion
        // since the t of the last sample doesn't match the t of the newest
        // sample of the new data.
        if (userdata.voices[0].samples.size() < fs / 10)
            makeSamples(userdata.voices[0].samples, noteFrequncy(Note::C, 4));

        if (userdata.voices[1].samples.size() < fs / 10)
            makeSamples(userdata.voices[1].samples, noteFrequncy(Note::D, 4));

        if (userdata.voices[2].samples.size() < fs / 10)
            makeSamples(userdata.voices[2].samples, noteFrequncy(Note::E, 4));

        if (userdata.voices[3].samples.size() < fs / 10)
            makeSamples(userdata.voices[3].samples, noteFrequncy(Note::F, 4));




        // TODO: Is it easier to just serve up more audio on demand
        // using a callback?
        // if (SDL_GetQueuedAudioSize(device) < fs)
        // {
        //     SDL_QueueAudio(device, sampleData, numSamples);
        // }


        SDL_SetRenderDrawColor(pRenderer, 0x64, 0x95, 0xed, 0xff);
        // SDL_SetRenderDrawColor(pRenderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderClear(pRenderer);


        // TODO: Could probably just draw the entire keyboard,
        // with certain keys lit up as needed.
        const uint32_t WHITE_KEY_WIDTH = 32;
        const uint32_t WHITE_KEY_HEIGHT = WHITE_KEY_WIDTH * 6;
        auto drawWhiteKey = [&pRenderer](uint32_t x, uint32_t y) {
            SDL_Rect rect;

            SDL_SetRenderDrawColor(pRenderer, 255, 253, 247, 0xff);
            rect.x = x;
            rect.y = y;
            rect.w = WHITE_KEY_WIDTH;
            rect.h = WHITE_KEY_HEIGHT;
            SDL_RenderFillRect(pRenderer, &rect);
        };

        const uint32_t BLACK_KEY_WIDTH = WHITE_KEY_WIDTH / 2;
        const uint32_t BLACK_KEY_HEIGHT = 2 * WHITE_KEY_WIDTH / 3;
        auto drawBlackKey = [&pRenderer](uint32_t x, uint32_t y) {
            SDL_Rect rect;

            SDL_SetRenderDrawColor(pRenderer, 20, 20, 20, 0xff);
            rect.x = x;
            rect.y = y;
            rect.w = BLACK_KEY_WIDTH;
            rect.h = BLACK_KEY_HEIGHT;
            SDL_RenderFillRect(pRenderer, &rect);
        };


        SDL_Rect rect;
        rect.x = 32;
        rect.y = 480-32;
        rect.w = 14 * WHITE_KEY_WIDTH;
        rect.h = 64 + WHITE_KEY_HEIGHT;
        SDL_SetRenderDrawColor(pRenderer, 140, 140, 140, 0xff);
        // SDL_RenderFillRect(pRenderer, &rect);

        for (int i = 0; i < 15; i++)
        {
            drawWhiteKey(64 + (WHITE_KEY_WIDTH + 2) * i, 480);
        }

        for (int i = 0; i < 15; i++)
        {
            drawBlackKey(64 + (3 * WHITE_KEY_WIDTH / 4) + WHITE_KEY_WIDTH * i, 480);
        }






        SDL_RenderCopy(pRenderer, msgTexture, NULL, &msgRect);

        SDL_RenderPresent(pRenderer);

        // TODO: Remove this, most likely.
        // SDL_Delay(16);
    }


    SDL_DestroyTexture(msgTexture);

    SDL_CloseAudio();
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();

    return 0;

}
