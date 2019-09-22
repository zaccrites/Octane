
#include <iostream>
#include <limits>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "Vsynth.h"

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

double noteFrequency(Note note, uint8_t octave)
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



const uint32_t WINDOW_WIDTH = 640;
const uint32_t WINDOW_HEIGHT = 480;


int main()
{

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cerr << "Failed to init SDL" << std::endl;
        return 1;
    }

    // if (TTF_Init() != 0)
    // {
    //     std::cerr << "Failed to init SDL TTF" << std::endl;
    //     return 1;
    // }

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


    // constexpr uint32_t SAMPLE_RATE = 44100;
    constexpr uint32_t SAMPLE_RATE = 65536;


    // https://wiki.libsdl.org/SDL_AudioSpec
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID device;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S32;
    want.channels = 1;
    want.samples = 512;
    // want.callback = audio_callback;
    // want.userdata = &userdata;
    want.callback = NULL;
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
    //
    // The delay isn't a problem running natively on macOS,
    // and I expect it wouldn't happen on native Windows or Linux either.
    // It's almost certainly the VMware VM adding that delay.

    // SDL_QueueAudio(device, sampleData, numSamples);
    // SDL_PauseAudioDevice(device, 0);
    // SDL_Delay(1000);
    // SDL_PauseAudioDevice(device, 1);
    // SDL_Delay(3000);
    // SDL_PauseAudioDevice(device, 0);




    // TODO: Wrap in class
    Vsynth synth;
    auto tick = [&synth]() -> void {
        synth.i_Clock = 0;
        synth.eval();
        synth.i_Clock = 1;
        synth.eval();
    };
    auto reset = [&synth, &tick]() -> void {
        synth.i_Reset = 1;
        tick();
        synth.i_Reset = 0;
    };

    reset();



    auto makeFreq = [](double f) -> uint32_t {
        // Convert raw Hz to multiple of 2^-8 Hz
        return static_cast<uint32_t>(f * (1 << 8));
    };





    auto toFixed = [FRAC_BITS=8](double x) -> uint32_t {
        return static_cast<uint32_t>(x * (1 << FRAC_BITS) + 0.5);
    };


    // TODO
    // I may be missing out on modulation range because anything over 1.0
    // here causes the output of Op1 to overflow the 24 bits.


    const uint16_t VOICE_ALGORITHM     = 0x00;
    const uint16_t VOICE_OP1_AMPLITUDE = 0x01;
    const uint16_t VOICE_OP1_FREQUENCY = 0x02;
    const uint16_t VOICE_OP2_AMPLITUDE = 0x03;
    const uint16_t VOICE_OP2_FREQUENCY = 0x04;
    const uint16_t VOICE_KEYON         = 0x05;

    auto writeRegister = [&synth, &tick](uint8_t voiceNum, uint8_t registerNum, int32_t registerValue) {
        synth.i_RegisterWriteEnable = 1;
        synth.i_RegisterNumber = (voiceNum << 6) | registerNum;
        synth.i_RegisterValue = registerValue;
        tick();
        synth.i_RegisterWriteEnable = 0;
    };

    // writeRegister(1, VOICE_ALGORITHM, 1);
    // writeRegister(1, VOICE_OP1_AMPLITUDE, toFixed(0.50));
    // // writeRegister(1, VOICE_OP1_AMPLITUDE, toFixed(1.0 / 6.0));
    // writeRegister(1, VOICE_OP1_FREQUENCY, makeFreq(350));
    // writeRegister(1, VOICE_OP2_AMPLITUDE, toFixed(0.50));
    // // writeRegister(1, VOICE_OP2_AMPLITUDE, toFixed(1.0 / 6.0));
    // writeRegister(1, VOICE_OP2_FREQUENCY, makeFreq(440));

    // writeRegister(2, VOICE_ALGORITHM, 1);
    // writeRegister(2, VOICE_OP1_AMPLITUDE, toFixed(0.50));
    // // writeRegister(2, VOICE_OP1_AMPLITUDE, toFixed(2.0 / 6.0));
    // writeRegister(2, VOICE_OP1_FREQUENCY, makeFreq(880));
    // writeRegister(2, VOICE_OP2_AMPLITUDE, toFixed(0.50));
    // // writeRegister(2, VOICE_OP2_AMPLITUDE, toFixed(2.0 / 6.0));
    // writeRegister(2, VOICE_OP2_FREQUENCY, makeFreq(700));

    // writeRegister(VOICE1_KEYON, 1);
    // writeRegister(VOICE2_KEYON, 1);

    for (uint8_t voiceNum = 1; voiceNum <= 8; voiceNum++)
    {
        writeRegister(voiceNum, VOICE_ALGORITHM, 1);
        writeRegister(voiceNum, VOICE_OP1_AMPLITUDE, toFixed(1.00));
        // writeRegister(voiceNum, VOICE_OP1_FREQUENCY, ...);
        writeRegister(voiceNum, VOICE_OP2_AMPLITUDE, toFixed(0.00));
        // writeRegister(voiceNum, VOICE_OP2_FREQUENCY, ...);


    }

    // writeRegister(1, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::C, 4)));
    // writeRegister(2, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::D, 4)));
    // writeRegister(3, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::E, 4)));
    // writeRegister(4, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::F, 4)));
    // writeRegister(5, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::G, 4)));
    // writeRegister(6, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::A, 5)));
    // writeRegister(7, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::B, 5)));
    // writeRegister(8, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::C, 5)));

    writeRegister(1, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::G, 4)));
    writeRegister(2, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::A, 4)));
    writeRegister(3, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::B, 4)));
    writeRegister(4, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::C, 5)));
    writeRegister(5, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::D, 5)));
    writeRegister(6, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::E, 5)));
    writeRegister(7, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::Fs, 5)));
    writeRegister(8, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::G, 5)));

    // For the graph visualization, it would also be nice to be able
    // to see the plain sine wave of the carrier as well as the modulator.

    // The keyboard interface will provide a fundamental frequency,
    // and I'll need a "coarse frequency" or whatever, similar to the
    // DX11 to adjust (including transposition).
    //
    // It's probably more flexible to allow any frequency to be applied
    // to the operators, and software controlling the synth can abstract
    // away the "coarse adjust" and key frequency and other such concepts.

    constexpr double length = 10.0;
    constexpr size_t NUM_SAMPLES = SAMPLE_RATE * length;
    int32_t samples[NUM_SAMPLES];

    FILE* csv = fopen("data.csv", "w");
    fprintf(csv, "i,sample\n");
    for (size_t i = 0; i < NUM_SAMPLES; i++) {

        auto sweep = [i](double start, double end) -> double {
            const double a = static_cast<double>(i) / static_cast<double>(NUM_SAMPLES);
            return start * (1.0 - a) + end * a;
        };


        bool skip = false;
        for (uint8_t vi = 0; vi < 8; vi++)
        {
            if (i == vi * NUM_SAMPLES / 8)
            {

                writeRegister(vi + 1, VOICE_KEYON, 1);
                skip = true;
                break;
            }
        }
        if ( ! skip) tick();

        // Extend 24-bit table to the full range of 32 bits
        int32_t sample = synth.o_Sample * (1 << 8);
        samples[i] = sample;

        // if (i < 1000)
        {
            fprintf(csv, "%zu,%d \n", i, sample);
        }
    }
    fclose(csv);

    // printf("Exiting early \n");
    // return 0;

    SDL_QueueAudio(device, samples, NUM_SAMPLES * sizeof(samples[0]));
    SDL_PauseAudioDevice(device, 0);



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
                        {
                            running = false;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        SDL_SetRenderDrawColor(pRenderer, 0x64, 0x95, 0xed, 0xff);
        SDL_RenderClear(pRenderer);
        SDL_RenderPresent(pRenderer);
        SDL_Delay(16);
    }


    SDL_CloseAudio();
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();

    return 0;


    // Next steps:
    /*

        - Output results from Sine wave generator into SDL so I can listen
         to them. Do they sound right? Is there any crossover distortion?
        - Refine table generator. Is the function right?
        - Verify multiplier at boundary values

        - Implement a single "operator" using directly input registers.
        - Allow writing registers using SPI-like interface
        - Implement a two-operator configuration to do FM synthesis
          (or additive synthesis in second mode, see YM3812).

        - Implement additional voices. Do voices share operator registers
          or all have their own? The voices will at least have their own
          fundamental frequency register value.

        - Implement four operators per voice.

        - Move from SDL simulation to FPGA using PWM


        - Still not sure about the configuration interface.
          Something on an external interface would be cool.
          Maybe something on a web page? Or my iPad?

    */



}
