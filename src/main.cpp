
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

    void writeOperatorRegister(uint16_t voiceNum, uint16_t operatorNum, uint16_t parameter, uint16_t value)
    {
        // FUTURE: This may need to be reworked if I ever want e.g.
        // 8 operators and 32 voices, as Voice-level configuration
        // won't be able to go in operator slot 0 any more with only 3 bits.
        const uint16_t registerNumber =
            ((voiceNum & 0x001f) << 11) |        // 5 bit voice
            ((operatorNum & 0x0007) << 8) |      // 3 bit operator
            (parameter & 0x00ff);                // 8 bit parameter
        writeRegister(registerNumber, value);
    }

    void writeVoiceRegister(uint16_t voiceNum, uint16_t parameter, uint16_t value)
    {
        writeOperatorRegister(voiceNum, 0, parameter, value);
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

public:
    static const uint16_t VOICE_PARAM_KEYON  { 0x00 };
    static const uint16_t VOICE_PARAM_ALGORITHM  { 0x01 };

    static const uint16_t OP_PARAM_PHASE_STEP  { 0x00 };
    static const uint16_t OP_PARAM_WAVEFORM    { 0x01 };
    static const uint16_t OP_PARAM_ENVELOPE_LEVEL  { 0x02 };


    static const uint16_t OP_WAVEFORM_SINE  { 0x0000 };
    static const uint16_t OP_WAVEFORM_SQUARE { 0x0001 };

private:
    Vsynth m_Synth;
    std::queue<int16_t> m_SampleBuffer;

};



// Q0.16
int16_t toFixed(double x) {
    // Not sure how to handle this properly (when there are no integer bits
    // but I want the possible values to go up to 1.000).
    // Very slightly less than 100% is okay I guess.
    // if (x >= 1.0) return 0x7fff;

    auto result = static_cast<int16_t>(x * 0x7fff);
    // printf("Calculated fixed result of %u for input of %f \n", result, x);
    return result;
}


int main()
{
    Synth synth;
    synth.reset();





    // auto toFixed = [FRAC_BITS=16](double x) -> uint16_t {
    //     return static_cast<uint16_t>(x * (1 << FRAC_BITS) + 0.5);
    // };

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
            uint16_t phaseStep;
            uint16_t outputLevel;

            if (voiceNum == 1)
            {
                if (operatorNum == 5)
                {
                    phaseStep = phaseStepForFrequency(220.0);
                    outputLevel = toFixed(1.0);
                }
                else if (operatorNum == 6)
                {
                    phaseStep = phaseStepForFrequency(440.0);
                    outputLevel = toFixed(1.0);
                }
                else
                {
                    phaseStep = phaseStepForFrequency(1000.0);
                    outputLevel = toFixed(0.0);
                }
            }
            else
            {
                // phaseStep = phaseStepForFrequency(110.0);
                // if (operatorNum == 5)
                // {
                //     outputLevel = toFixed(0.5);
                // }
                // else
                // {
                //     outputLevel = toFixed(0.0);
                // }

                phaseStep = phaseStepForFrequency(1000.0);
                outputLevel = toFixed(0.0);
            }


            // phaseStep = phaseStepForFrequency(100.0 * voiceNum);
            // outputLevel = toFixed(1.0 / 16.0);



            // if (voiceNum == 15)
            // {
            //     if (operatorNum == 5)
            //     {
            //         // Modulator
            //         outputLevel = toFixed(0.0);
            //         phaseStep = phaseStepForFrequency(220.0);
            //     }
            //     else if (operatorNum == 6)
            //     {
            //         // Carrier
            //         outputLevel = toFixed(0.0);
            //         phaseStep = phaseStepForFrequency(440.0);
            //     }
            //     else
            //     {
            //         // Disabled
            //         outputLevel = toFixed(0.0);
            //         phaseStep = phaseStepForFrequency(1000.0);
            //     }
            // }
            // else
            // {

            //     // I suspect a pipeline bug is why they're all zero
            //     // outputLevel = toFixed(0.1 / 8.0);
            //     phaseStep = phaseStepForFrequency(600.0 + 20.0 * (voiceNum - 1));

            //     outputLevel = toFixed(0.0);
            //     // phaseStep = phaseStepForFrequency(1000.0);
            // }

            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_WAVEFORM, Synth::OP_WAVEFORM_SINE);
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_PHASE_STEP, phaseStep);
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_ENVELOPE_LEVEL, outputLevel);
        }

        const uint16_t algorithmNumber = 1;
        synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_ALGORITHM, algorithmNumber - 1);
        synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_KEYON, true);
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

        // if (i < 1000)
        // if (i < 300)
        // if (250 <= i && 300 >= i)
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
    SDL_Delay(1000);

    SDL_CloseAudio();
    SDL_Quit();
    return 0;

}
