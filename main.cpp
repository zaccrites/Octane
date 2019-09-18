
#include <stdint.h>
#include <iostream>
#include <queue>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


const uint32_t WINDOW_WIDTH = 1024;
const uint32_t WINDOW_HEIGHT = 768;


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


// TODO: Use a lambda
void audio_callback (void* pUserdata, uint8_t* pStream, int length)
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

    const uint32_t fs = 44100;
    const uint32_t numSamples = fs * 120;
    // int16_t sampleData[numSamples];


    PlaybackUserData userdata;
    userdata.voices[0].playing = false;
    userdata.voices[1].playing = false;
    userdata.voices[2].playing = false;
    userdata.voices[3].playing = false;


    auto makeSamples = [](std::queue<int16_t>& rSampleData, double f) {
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

            double value = A * std::sin(C * t + I * std::sin(M * t));

            rSampleData.push(static_cast<int16_t>(0x7fff * value));
        }
    };
    makeSamples(userdata.voices[0].samples, 350.0);
    makeSamples(userdata.voices[1].samples, 440.0);
    makeSamples(userdata.voices[2].samples, 220.0);
    makeSamples(userdata.voices[3].samples, 1000.0);



    // https://wiki.libsdl.org/SDL_AudioSpec
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID device;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = fs;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 128;
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
