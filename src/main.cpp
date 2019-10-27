
#include <printf.h>
#include <iostream>
#include <fstream>
#include <algorithm>  // for std::find
#include <optional>

#include <SDL2/SDL.h>

#include "debug.hpp"
#include "synth.hpp"
#include "PatchConfig.hpp"


const uint32_t SAMPLE_FREQUENCY = 44100;



int16_t toFixed(double x) {
    // Not sure how to handle this properly (when there are no integer bits
    // but I want the possible values to go up to 1.000).
    // Very slightly less than 100% is okay I guess.
    // if (x >= 1.0) return 0x7fff;

    auto result = static_cast<int16_t>(x * 0x7fff);
    // printf("Calculated fixed result of %u for input of %f \n", result, x);
    return result;
}


uint16_t phaseStepForFrequency(double frequency) {
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
}



int main(int argc, const char** argv)
{
    setup_debug_handlers();

    auto getOption = [argv, argc](const std::string& option) -> std::optional<std::string> {
        const char** end = argv + argc;
        const char** it = std::find(argv, end, option);
        if (it != end && ++it != end)
        {
            return *it;
        }
        return {};
    };

    auto isOptionPresent = [argv, argc](const std::string& option) -> bool {
        const char** end = argv + argc;
        const char** it = std::find(argv, end, option);
        return it != end;
    };

    const auto playAudio = isOptionPresent("play");

    const auto patchConfigPath = getOption("-p");
    if ( ! patchConfigPath)
    {
        std::cerr << "ERROR: Must provide path to patch config file" << std::endl;
        return 1;
    }

    auto patchConfig = PatchConfig::load(*patchConfigPath);
    std::cout << "Setting up patch " << patchConfig.getName() << std::endl;

    Synth synth;
    synth.reset();


    for (uint16_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        double noteBaseFrequency;
        // if (voiceNum < 16)
        // {
        //     // noteBaseFrequency = 500.0;
        //     noteBaseFrequency = 350.0;
        // }
        // else
        {
            // noteBaseFrequency = 1000.0;
            noteBaseFrequency = 400.0;
        }

        // noteBaseFrequency = 100.0 * (1 + voiceNum);


        // auto algorithmNumber = patchConfig.getAlgorithm();
        // synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_ALGORITHM, algorithmNumber - 1);

        /*
        "Algorithm 1"

        1   5
        |   |
        2   6
        |   |
        3   7
        |   |
        4---8

        Alghorithm bits:

        rgfe dcba

        r - Operator is a carrier
        g - Modulated by OP7
        f - Modulated by OP6
        e - Modulated by OP5
        d - Modulated by OP4
        c - Modulated by OP3
        b - Modulated by OP2
        a - Modulated by OP1

        */


        // auto makeAlgorithmWord = [](uint8_t modulation, bool isCarrier, uint8_t numCarriers)
        uint16_t algorithmWords[8] = {
            //   7654321
            //xx mmmmmmm xxxx c nnn
            0b00'0000000'0000'0'000,  // OP1
            0b00'0000000'0000'0'000,  // OP2
            0b00'0000000'0000'0'000,  // OP3
            0b00'0000000'0000'0'000,  // OP4
            0b00'0000000'0000'0'000,  // OP5
            0b00'0000000'0000'0'000,  // OP6
            0b00'0000000'0000'0'000,  // OP7
            0b00'1111111'0000'1'000,  // OP8

        };


        synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_NOTEON, false);

        for (uint16_t opNum = 0; opNum < 8; opNum++)
        {
            auto opConfig = patchConfig.getOperatorConfig(opNum);

            // TODO: Use JSON
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ALGORITHM_HIGH, algorithmWords[opNum] >> 8);
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ALGORITHM_LOW, algorithmWords[opNum] & 0xff);

            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_L1, opConfig.getEnvelopeL1());
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_L2, opConfig.getEnvelopeL2());
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_L3, opConfig.getEnvelopeL3());
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_L4, opConfig.getEnvelopeL4());

            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_R1, opConfig.getEnvelopeR1());
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_R2, opConfig.getEnvelopeR2());
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_R3, opConfig.getEnvelopeR3());
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_R4, opConfig.getEnvelopeR4());

            // uint16_t waveform;
            // switch (opConfig.getWaveform())
            // {
            //     case OperatorConfig::Waveform::Square:
            //         waveform = Synth::OP_WAVEFORM_SQUARE;
            //         break;

            //     case OperatorConfig::Waveform::Sine:
            //     default:
            //         waveform = Synth::OP_WAVEFORM_SINE;
            //         break;
            // }
            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_WAVEFORM, waveform);

            uint16_t phaseStep = phaseStepForFrequency(noteBaseFrequency * opConfig.getFrequencyRatio());
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_PHASE_STEP_HIGH, phaseStep >> 8);
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_PHASE_STEP_LOW, phaseStep & 0xff);
        }
    }

    for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        // if (voiceNum <= 1)
        {
            synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_NOTEON, true);
        }
    }


    // printf("Registers: \n");
    // printf("Phase Step: \n");
    // for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum++)
    // {
    //     if ( ! (voiceNum == 0 || voiceNum == 31)) continue;

    //     const bool noteOn = synth.getRawModel().synth__DOT__r_NoteOn[voiceNum];
    //     printf("  %d NoteOn = %d \n", voiceNum, noteOn);

    //     for (uint8_t opNum = 0; opNum < 8; opNum++)
    //     {
    //         const uint8_t index = (opNum << 5) | voiceNum;
    //         const uint16_t phaseStep = synth.getRawModel().synth__DOT__r_PhaseStep[index];
    //         printf("  %d.%d phaseStep = 0x%04x \n", voiceNum, opNum, phaseStep);
    //     }
    // }



    double seconds = playAudio ? 2.0 : 0.25;
    auto& rBuffer = synth.getSampleBuffer();

    double noteOn = true;
    while (rBuffer.size() < static_cast<uint32_t>(SAMPLE_FREQUENCY * seconds))
    {
        double t = static_cast<double>(rBuffer.size()) / static_cast<double>(SAMPLE_FREQUENCY);
        // if (noteOn && t >= seconds - 1.0)
        // {
        //     for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum++)
        //     {
        //         // if (voiceNum < 16)
        //         {
        //             synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_NOTEON, false);
        //         }
        //     }
        //     noteOn = false;
        // }

        synth.tick();

        // printf("rBuffer.size() = %d \n", rBuffer.size());

    }


    if (playAudio)
    {

        // TODO: Graphics?
        // if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        if (SDL_Init(SDL_INIT_AUDIO) != 0)
        {
            std::cerr << "Failed to init SDL" << std::endl;
            return 1;
        }

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


        // TODO
        SDL_PauseAudioDevice(device, 0);
        SDL_Delay(static_cast<uint32_t>(seconds * 1000));

        SDL_CloseAudio();
        SDL_Quit();

    }

    return 0;
}
