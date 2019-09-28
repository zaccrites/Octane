
#include <printf.h>
#include <iostream>
#include <queue>

#include <SDL2/SDL.h>
#include "Vsynth.h"


class Synth
{
public:

    void tick()
    {
        m_Synth.i_Clock = 0;
        m_Synth.eval();
        m_Synth.i_Clock = 1;
        m_Synth.eval();

        if (m_Synth.o_SampleReady)
        {
            m_SampleBuffer.push(m_Synth.o_Sample);
        }
    }

    void reset()
    {
        m_Synth.i_Reset = 1;
        tick();
        m_Synth.i_Reset = 0;
    }

    void writeRegister(uint16_t registerNumber, uint16_t registerValue)
    {
        m_Synth.i_RegisterNumber = registerNumber;
        m_Synth.i_RegisterValue = registerValue;
        m_Synth.i_RegisterWriteEnable = 1;
        tick();
        m_Synth.i_RegisterWriteEnable = 0;
    }

    void writeSampleBytes(uint8_t* pRawStream, size_t number)
    {
        while (m_SampleBuffer.size() < number)
        {
            tick();
        }

        int16_t* pStream = reinterpret_cast<int16_t*>(pRawStream);
        for (size_t i = 0; i < number / sizeof(int16_t); i++)
        {
            pStream[i] = m_SampleBuffer.front();
            m_SampleBuffer.pop();
        }
    }

    // TODO: Find a better way
    std::queue<int16_t> cloneSampleBuffer() const
    {
        return m_SampleBuffer;
    }

    size_t getNumSamplesBuffered() const
    {
        return m_SampleBuffer.size();
    }

private:
    Vsynth m_Synth;
    std::queue<int16_t> m_SampleBuffer;

};


int main()
{
    Synth synth;
    synth.reset();


    const uint32_t SAMPLE_FREQUENCY = 44100;

    auto phaseStepForFrequency = [](double frequency) -> uint16_t {
        // Formula:
        // phaseStep = 2^N * f / FS
        // where N is the number of bits of phase accumulation
        // FS is the sample frequency
        // and f is the desired tone frequency
        return static_cast<uint16_t>(
            static_cast<double>(1 << 16) *
            frequency /
            static_cast<double>(SAMPLE_FREQUENCY)
        );
    };

    // Set phase step
    for (uint16_t voiceNum = 1; voiceNum <= 16; voiceNum++)
    {
        for (uint16_t operatorNum = 1; operatorNum <= 6; operatorNum++)
        {
            uint16_t registerNumber =
                (voiceNum << 12) | (operatorNum << 9) | 0x0000;
            const uint16_t phaseStep = (voiceNum % 2 == 0)
                ? phaseStepForFrequency(440.0)
                // : phaseStepForFrequency(350.0);
                : phaseStepForFrequency(220.0);
            synth.writeRegister(registerNumber, phaseStep);


            // Use a square wave
            registerNumber =
                (voiceNum << 12) | (operatorNum << 9) | 0x0001;
            synth.writeRegister(registerNumber, 1);

        }
    }



    // TODO: Graphics?
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        std::cerr << "Failed to init SDL" << std::endl;
        return 1;
    }





    uint32_t x = 0;
    while (synth.getNumSamplesBuffered() < SAMPLE_FREQUENCY)
    {
        synth.tick();

        if (x > SAMPLE_FREQUENCY * 100)
        {
            printf("Exceeded clock %d, aborting \n", x);
        }
        x += 1;
    }
    printf("it took %d clocks \n", x);

    size_t i = 0;
    auto samples = synth.cloneSampleBuffer();
    FILE* csv = fopen("data.csv", "w");
    while ( ! samples.empty())
    {
        int16_t sample = samples.front();
        samples.pop();

        if (i < 1000)
            fprintf(csv, "%zu,%d\n", i, sample);
        i += 1;
    }
    fclose(csv);



    // https://wiki.libsdl.org/SDL_AudioSpec
    SDL_AudioSpec want, have;
    SDL_AudioDeviceID device;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_FREQUENCY;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 1024;
    want.userdata = &synth;
    want.callback = [](void* pUserdata, uint8_t* pBuffer, int length) {
        Synth* pSynth = static_cast<Synth*>(pUserdata);
        pSynth->writeSampleBytes(pBuffer, length);
    };
    device = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (device == 0)
    {
        std::cerr << "Failed to init SDL audio" << std::endl;
        return 1;
    }

    SDL_PauseAudioDevice(device, 0);
    SDL_Delay(2000);

    SDL_CloseAudio();
    SDL_Quit();
    return 0;

}
