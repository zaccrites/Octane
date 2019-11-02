
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


    // uint16_t phaseStep0 = phaseStepForFrequency(440.0);
    // synth.writeOperatorRegister(0, 7, Synth::OP_PARAM_PHASE_STEP, phaseStep0);
    // synth.spiSendReceive();
    // synth.spiSendReceive();

    // // ???
    // synth.spiSendReceive();
    // return 0;




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
            noteBaseFrequency = 523.251;  // C5
        }
        // noteBaseFrequency = 100.0 * (1 + voiceNum);
        // noteBaseFrequency = 440.0;


        // auto makeAlgorithmWord = [](uint8_t modulation, bool isCarrier, uint8_t numCarriers)
        uint16_t algorithmWords[8] = {
            //       7654321
            //xxxxxx mmmmmmm nnn c
            0b000000'0000000'000'0,  // OP1
            0b000000'0000001'000'1,  // OP2
            0b000000'0000000'000'0,  // OP3
            0b000000'0000000'000'0,  // OP4
            0b000000'0000000'000'0,  // OP5
            0b000000'0000000'000'0,  // OP6
            0b000000'0000000'000'0,  // OP7
            0b000000'0000000'000'0,  // OP8

            // 0b000000'0000000'000'0,  // OP1
            // 0b000000'0000000'000'0,  // OP2
            // 0b000000'0000000'000'0,  // OP3
            // 0b000000'0000000'000'0,  // OP4
            // 0b000000'0000000'000'0,  // OP5
            // 0b000000'0000000'000'0,  // OP6
            // 0b000000'0000000'001'1,  // OP7
            // 0b000000'0000000'001'1,  // OP8
        };

        synth.setNoteOn(voiceNum, false);

        for (uint16_t opNum = 0; opNum < 8; opNum++)
        {
            auto opConfig = patchConfig.getOperatorConfig(opNum);

            // TODO: Use JSON
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ALGORITHM, algorithmWords[opNum]);

            uint16_t phaseStep = phaseStepForFrequency(noteBaseFrequency * opConfig.getFrequencyRatio());
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_PHASE_STEP, phaseStep);

            bool isCarrier = algorithmWords[opNum] & 0x0001;

            // TODO
            auto modulationIndexForLevel = [](uint16_t level) -> double {
                // Vary linearly up to I=15
                return level * 15.0 / 1000.0;
            };

            // TODO: Change actual levels to match modulation index (function of frequency and set level), if is a modulator
            auto fixOperatorLevel = [modulationIndex, phaseStep, isCarrier](uint16_t level) -> uint16_t {
                if (isCarrier)
                {
                    double multiplier = static_cast<double>(level) / 1000.0;
                    return static_cast<uint16_t>(0x3fff * multiplier);
                }
                else
                {
                    return static_cast<uint16_t>(modulationIndex * phaseStep);
                }
            };
            // ... that will affect the rate as well, since the time to reach the level should be the same regardless of note frequency played
            auto fixOperatorRate = [modulationIndex, phaseStep, isCarrier](uint16_t rate) -> uint16_t {

                if (isCarrier)
                {
                    double multiplier = static_cast<double>(rate) / 1000.0;
                    return static_cast<uint16_t>(0x0fff * multiplier);
                }
                else
                {
                    // TODO
                    return 0x0fff;
                    // return modulationIndex * phaseStep / 4;
                }
            };

            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_ATTACK_LEVEL, fixOperatorLevel(opConfig.getAttackLevel()));
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_SUSTAIN_LEVEL, fixOperatorLevel(opConfig.getSustainLevel()));
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_ATTACK_RATE, fixOperatorRate(opConfig.getAttackRate()));
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_DECAY_RATE, fixOperatorRate(opConfig.getDecayRate()));
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_RELEASE_RATE, fixOperatorRate(opConfig.getReleaseRate()));


        }
    }

    for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        if (voiceNum == 0)
        {
            synth.setNoteOn(voiceNum, true);
        }
    }

    double seconds = playAudio ? 2.0 : 0.3;
    auto& rBuffer = synth.getSampleBuffer();

    // double noteOn = false;
    while (rBuffer.size() < static_cast<uint32_t>(SAMPLE_FREQUENCY * seconds))
    {
        double t = static_cast<double>(rBuffer.size()) / static_cast<double>(SAMPLE_FREQUENCY);
        // double onTime = seconds * 0.0;
        // double offTime = seconds * 0.8;
        // if ( ! noteOn && t > onTime && t < offTime)
        // {
        //     noteOn = true;
        //     synth.setNoteOn(0, true);
        // }
        // else if (noteOn && t > offTime)
        // {
        //     noteOn = false;
        //     synth.setNoteOn(0, false);
        // }

        // for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum++)
        // {
        //     double voiceOnTime = voiceNum * (seconds / 32.0);
        //     if (t > voiceOnTime && ! synth.getNoteOn(voiceNum))
        //     {
        //         printf("t = %f, setting voice %d on \n", t, voiceNum);
        //         synth.setNoteOn(voiceNum, true);
        //     }
        // }

        // for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum += 2)
        // {
        //     double voiceOnTime = voiceNum * (seconds / 32.0);
        //     if (t > voiceOnTime && ! synth.getNoteOn(voiceNum))
        //     {
        //         synth.setNoteOn(voiceNum, true);
        //         synth.setNoteOn(voiceNum + 1, true);
        //     }
        // }


        synth.spiSendReceive();

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
