
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
    static const uint16_t VOICE_PARAM_AMPLITUDE_ADJUST { 0x02 };

    static const uint16_t OP_PARAM_PHASE_STEP  { 0x00 };
    static const uint16_t OP_PARAM_WAVEFORM    { 0x01 };

    static const uint16_t OP_PARAM_ATTACK_LEVEL  { 0x03 };
    static const uint16_t OP_PARAM_SUSTAIN_LEVEL  { 0x04 };
    static const uint16_t OP_PARAM_ATTACK_RATE  { 0x05 };
    static const uint16_t OP_PARAM_DECAY_RATE  { 0x06 };
    static const uint16_t OP_PARAM_RELEASE_RATE  { 0x07 };


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


uint16_t carriersForAlgorithm(uint16_t algorithmNumber)
{
    switch (algorithmNumber)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            return 2;

        case 5:
        case 6:
            return 3;

        case 7:
        case 8:
        case 9:
            return 2;

        case 10:
        case 11:
            return 2;

        case 12:
        case 13:
            return 2;

        case 14:
        case 15:
            return 2;

        case 16:
        case 17:
        case 18:
            return 1;

        case 19:
        case 20:
            return 3;

        case 21:
        case 22:
        case 23:
            return 4;

        case 24:
        case 25:
            return 5;

        case 26:
        case 27:
            return 3;

        case 28:
            return 3;

        case 29:
        case 30:
            return 4;

        case 31:
            return 5;

        case 32:
            return 6;


        default:
            return 1;
    }
}


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
            auto setEnvelope = [&synth, voiceNum, operatorNum](
                double attackLevel,
                double sustainLevel,
                double attackRate,
                double decayRate,
                double releaseRate
            ) {
                synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_ATTACK_LEVEL, toFixed(attackLevel));
                synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_SUSTAIN_LEVEL, toFixed(sustainLevel));
                synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_ATTACK_RATE, toFixed(attackRate));
                synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_DECAY_RATE, toFixed(decayRate));
                synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_RELEASE_RATE, toFixed(releaseRate));
            };


            uint16_t phaseStep;

            if (voiceNum == 1)
            {
                switch (operatorNum)
                {
                case 5:
                    phaseStep = phaseStepForFrequency(220.0);
                    setEnvelope(
                        1.0,  // attack level
                        0.2,  // sustain level
                        0.001,  // attack rate
                        0.05,  // decay rate
                        0.005   // release rate
                    );
                    break;

                case 6:
                    phaseStep = phaseStepForFrequency(440.0);
                    setEnvelope(
                        1.0,  // attack level
                        0.7,  // sustain level
                        0.05,  // attack rate
                        0.0005,  // decay rate
                        0.005   // release rate
                    );
                    break;


                // case 2:
                //     phaseStep = phaseStepForFrequency(30.0);
                //     outputLevel = toFixed(0.5);

                // case 3:
                //     phaseStep = phaseStepForFrequency(175.0);
                //     outputLevel = toFixed(1.0);
                //     break;

                // case 4:
                //     phaseStep = phaseStepForFrequency(350.0);
                //     outputLevel = toFixed(1.0);
                //     break;

                default:
                    phaseStep = phaseStepForFrequency(1000.0);
                    setEnvelope(
                        0.0,  // attack level
                        0.0,  // sustain level
                        0.0,  // attack rate
                        0.0,  // decay rate
                        0.0   // release rate
                    );
                    break;

                }

            }
            else
            {
                phaseStep = phaseStepForFrequency(1000.0);
                setEnvelope(
                    0.0,  // attack level
                    0.0,  // sustain level
                    0.0,  // attack rate
                    0.0,  // decay rate
                    0.0   // release rate
                );
            }

            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_WAVEFORM, Synth::OP_WAVEFORM_SINE);
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_PHASE_STEP, phaseStep);
        }

        const uint16_t algorithmNumber = 1;
        synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_ALGORITHM, algorithmNumber - 1);

        // synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_AMPLITUDE_ADJUST, toFixed(1.0 / carriersForAlgorithm(algorithmNumber)));
        synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_AMPLITUDE_ADJUST, toFixed(1.0));


        if (voiceNum == 1)
        {
            synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_KEYON, true);
        }
        else
        {
            synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_KEYON, false);
        }

    }



    // TODO: Graphics?
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        std::cerr << "Failed to init SDL" << std::endl;
        return 1;
    }


    unsigned seconds = 3;
    while (synth.getNumSamplesBuffered() < SAMPLE_FREQUENCY * seconds)
    {

        if (synth.getNumSamplesBuffered() > SAMPLE_FREQUENCY * (seconds - 1))
        {
            synth.writeVoiceRegister(1, Synth::VOICE_PARAM_KEYON, false);
        }

        synth.tick();
    }

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
    SDL_Delay(seconds * 1000 + 500);

    SDL_CloseAudio();
    SDL_Quit();
    return 0;

}
