
#include <iostream>
#include <limits>
#include <queue>
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





class Synth
{
public:

    void tick()
    {
        m_Synth.i_Clock = 0;
        m_Synth.eval();
        m_Synth.i_Clock = 1;
        m_Synth.eval();

        getSample();
    }

    void reset()
    {
        m_Synth.i_Reset = 1;
        tick();
        m_Synth.i_Reset = 0;

        clearSampleBuffer();
    }

    void writeRegister(uint8_t voiceNum, uint8_t registerNum, int32_t registerValue)
    {
        // TODO: Use uint32_t for e.g. setting KeyOn for all voice atomically
        // TODO: Force user to specify register number explcitly, as I can
        //   imagine having some global registers in the 00xx area which
        //   aren't tied to a specific voice.

        m_Synth.i_RegisterWriteEnable = 1;
        m_Synth.i_RegisterNumber = (voiceNum << 6) | registerNum;
        m_Synth.i_RegisterValue = registerValue;
        tick();
        m_Synth.i_RegisterWriteEnable = 0;
    }

    void writeSampleBytes(uint8_t* pRawStream, size_t number)
    {
        int32_t* pStream = reinterpret_cast<int32_t*>(pRawStream);
        for (size_t i = 0; i < number / sizeof(int32_t); i++)
        {
            if (m_SampleBuffer.empty())
            {
                for (int j = 0; j < 16; j++) tick();
            }
            pStream[i] = m_SampleBuffer.front();
            m_SampleBuffer.pop();
        }
    }

private:
    void getSample()
    {
        // TODO: Check a "data is valid" flag?

        // Extend 24-bit table to the full range of 32 bits
        int32_t sample = m_Synth.o_Sample * (1 << 8);
        m_SampleBuffer.push(sample);
    }

    void clearSampleBuffer()
    {
        // https://stackoverflow.com/a/709161
        std::queue<int32_t> empty;
        std::swap(m_SampleBuffer, empty);
    }

private:
    Vsynth m_Synth;
    std::queue<int32_t> m_SampleBuffer;

};



void sdl_audio_callback(void* userdata, uint8_t* buffer, int length)
{
    Synth* pSynth = static_cast<Synth*>(userdata);
    pSynth->writeSampleBytes(buffer, length);
}


int main()
{

    Synth synth;
    synth.reset();

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
    want.samples = 1024;
    want.callback = sdl_audio_callback;
    want.userdata = &synth;
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


    // synth.writeRegister(1, VOICE_ALGORITHM, 1);
    // synth.writeRegister(1, VOICE_OP1_AMPLITUDE, toFixed(0.50));
    // // synth.writeRegister(1, VOICE_OP1_AMPLITUDE, toFixed(1.0 / 6.0));
    // synth.writeRegister(1, VOICE_OP1_FREQUENCY, makeFreq(350));
    // synth.writeRegister(1, VOICE_OP2_AMPLITUDE, toFixed(0.50));
    // // synth.writeRegister(1, VOICE_OP2_AMPLITUDE, toFixed(1.0 / 6.0));
    // synth.writeRegister(1, VOICE_OP2_FREQUENCY, makeFreq(440));

    // synth.writeRegister(2, VOICE_ALGORITHM, 1);
    // synth.writeRegister(2, VOICE_OP1_AMPLITUDE, toFixed(0.50));
    // // synth.writeRegister(2, VOICE_OP1_AMPLITUDE, toFixed(2.0 / 6.0));
    // synth.writeRegister(2, VOICE_OP1_FREQUENCY, makeFreq(880));
    // synth.writeRegister(2, VOICE_OP2_AMPLITUDE, toFixed(0.50));
    // // synth.writeRegister(2, VOICE_OP2_AMPLITUDE, toFixed(2.0 / 6.0));
    // synth.writeRegister(2, VOICE_OP2_FREQUENCY, makeFreq(700));

    // synth.writeRegister(VOICE1_KEYON, 1);
    // synth.writeRegister(VOICE2_KEYON, 1);

    for (uint8_t voiceNum = 1; voiceNum <= 8; voiceNum++)
    {
        synth.writeRegister(voiceNum, VOICE_ALGORITHM, 0);

        // synth.writeRegister(voiceNum, VOICE_ALGORITHM, 1);
        synth.writeRegister(voiceNum, VOICE_OP1_AMPLITUDE, toFixed(0.50));
        // synth.writeRegister(voiceNum, VOICE_OP1_FREQUENCY, makeFreq(440.0 / 4.0 * (1 << voiceNum)));
        synth.writeRegister(voiceNum, VOICE_OP2_AMPLITUDE, toFixed(0.50));
        // synth.writeRegister(voiceNum, VOICE_OP2_FREQUENCY, makeFreq(350.0 / 4.0 * (1 << voiceNum)));

    }


    synth.writeRegister(1, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::G, 4)));
    synth.writeRegister(2, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::A, 4)));
    synth.writeRegister(3, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::B, 4)));
    synth.writeRegister(4, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::C, 5)));
    synth.writeRegister(5, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::D, 5)));
    synth.writeRegister(6, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::E, 5)));
    synth.writeRegister(7, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::Fs, 5)));
    synth.writeRegister(8, VOICE_OP1_FREQUENCY, toFixed(noteFrequency(Note::G, 5)));

    synth.writeRegister(1, VOICE_OP2_FREQUENCY, toFixed(noteFrequency(Note::G, 5)));
    synth.writeRegister(2, VOICE_OP2_FREQUENCY, toFixed(noteFrequency(Note::A, 5)));
    synth.writeRegister(3, VOICE_OP2_FREQUENCY, toFixed(noteFrequency(Note::B, 5)));
    synth.writeRegister(4, VOICE_OP2_FREQUENCY, toFixed(noteFrequency(Note::C, 6)));
    synth.writeRegister(5, VOICE_OP2_FREQUENCY, toFixed(noteFrequency(Note::D, 6)));
    synth.writeRegister(6, VOICE_OP2_FREQUENCY, toFixed(noteFrequency(Note::E, 6)));
    synth.writeRegister(7, VOICE_OP2_FREQUENCY, toFixed(noteFrequency(Note::Fs, 6)));
    synth.writeRegister(8, VOICE_OP2_FREQUENCY, toFixed(noteFrequency(Note::G, 6)));



    // synth.writeRegister(1, VOICE_KEYON, 1);

    // For the graph visualization, it would also be nice to be able
    // to see the plain sine wave of the carrier as well as the modulator.

    // The keyboard interface will provide a fundamental frequency,
    // and I'll need a "coarse frequency" or whatever, similar to the
    // DX11 to adjust (including transposition).
    //
    // It's probably more flexible to allow any frequency to be applied
    // to the operators, and software controlling the synth can abstract
    // away the "coarse adjust" and key frequency and other such concepts.

    // constexpr double length = 10.0;
    // constexpr size_t NUM_SAMPLES = SAMPLE_RATE * length;
    // int32_t samples[NUM_SAMPLES];

    // FILE* csv = fopen("data.csv", "w");
    // fprintf(csv, "i,sample\n");
    // for (size_t i = 0; i < NUM_SAMPLES; i++) {

    //     auto sweep = [i](double start, double end) -> double {
    //         const double a = static_cast<double>(i) / static_cast<double>(NUM_SAMPLES);
    //         return start * (1.0 - a) + end * a;
    //     };


    //     // bool skip = false;
    //     // for (uint8_t vi = 0; vi < 8; vi++)
    //     // {
    //     //     if (i == vi * NUM_SAMPLES / 8)
    //     //     {

    //     //         writeRegister(vi + 1, VOICE_KEYON, 1);
    //     //         skip = true;
    //     //         break;
    //     //     }
    //     // }
    //     // if ( ! skip) tick();
    //     synth.tick();

    //     // Extend 24-bit table to the full range of 32 bits
    //     int32_t sample = synth.o_Sample * (1 << 8);
    //     samples[i] = sample;

    //     if (i < 1000)
    //     {
    //         fprintf(csv, "%zu,%d \n", i, sample);
    //     }
    // }
    // fclose(csv);

    // printf("Exiting early \n");
    // return 0;

    // SDL_QueueAudio(device, samples, NUM_SAMPLES * sizeof(samples[0]));



    SDL_PauseAudioDevice(device, 0);

    // TODO: Keep track of which key is allocated to which voice,
    // if there are more keys than voices.
    // bool keyon = false;
    bool VoiceKeyOn[8];
    for (int i = 0; i < 8; i++) VoiceKeyOn[i] = false;
    uint8_t VoiceKey_Scancodes[8] = {
        SDL_SCANCODE_Q,
        SDL_SCANCODE_W,
        SDL_SCANCODE_E,
        SDL_SCANCODE_R,
        SDL_SCANCODE_T,
        SDL_SCANCODE_Y,
        SDL_SCANCODE_U,
        SDL_SCANCODE_I,
    };

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
                        // case SDLK_q:
                        {
                            running = false;
                            break;
                        }

                        // case SDLK_a:
                        // {
                        //     keyon = ! keyon;
                        //     synth.writeRegister(1, VOICE_KEYON, keyon ? 1 : 0);
                        //     break;
                        // }
                    }
                    break;
                }
            }
        }

        SDL_LockAudioDevice(device);
        const uint8_t* pKeystate = SDL_GetKeyboardState(NULL);
        for (int i = 0; i < 8; i++)
        {
            if (pKeystate[VoiceKey_Scancodes[i]] != VoiceKeyOn[i])
            {
                VoiceKeyOn[i] = pKeystate[VoiceKey_Scancodes[i]];
                synth.writeRegister(i + 1, VOICE_KEYON, VoiceKeyOn[i] ? 1 : 0);
            }
        }
        for (int i = 0; i < 256; i++) synth.tick();
        SDL_UnlockAudioDevice(device);

        SDL_SetRenderDrawColor(pRenderer, 0x64, 0x95, 0xed, 0xff);
        SDL_RenderClear(pRenderer);
        SDL_RenderPresent(pRenderer);

        // Only delay if there are enough samples in the buffer?
        SDL_Delay(4);  // ???
    }


    SDL_CloseAudio();
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_Quit();

    return 0;


    // Next steps:
    /*

        - Implement ADSR envelopes for each operator
        - Improve the "API". I feel like the modulation index of the original
          paper is lost somewere in the modulating operator's amplitude
          or something. Is that right?
        - Implement feedback for operators
        - Implement four operators per voice
        - Either implement more keys on my fake keyboard, or interface
          with a real keyboard (either through MIDI or some hacked version)
        - Deal with the various TODOs littering the code.

        - Implement PWM operator waveform

        - Allow writing registers using SPI-like interface
        - Divide real clock so that the sample clock runs slower than
          the main clock?

        - Move from SDL simulation to FPGA using PWM

        - Still not sure about the configuration interface.
          Something on an external interface would be cool.
          Maybe something on a web page? Or my iPad?

    */



}
