
#include <printf.h>
#include <iostream>
#include <fstream>

#include <SDL2/SDL.h>

#include "synth.hpp"
// #include "server.hpp"

#include <thread>
#include <chrono>
#include <atomic>


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



// https://stackoverflow.com/a/3356421
#include <stdexcept>
#include <execinfo.h>
void terminate_handler()
{
    // On using addr2line:
    // https://stackoverflow.com/a/51597989
    //
    // Example: addr2line -afC -e ../synth-build/synth +0x69af

    std::cerr << "Printing stack trace of uncaught exception: \n";
    std::exception_ptr eptr = std::current_exception();
    try
    {
        // assert(eptr);
        std::rethrow_exception(eptr);
    }
    catch (const std::exception& e)
    {
        std::cerr << "what(): " << e.what() << "\n";
    }
    std::cerr << "------------------------------------------------------------"
              << std::endl;

    void *trace_elems[20];
    int trace_elem_count = backtrace(trace_elems, 20);
    char **stack_syms = backtrace_symbols(trace_elems, trace_elem_count);
    for (int i = 0; i < trace_elem_count; i++)
    {
        std::cout << stack_syms[trace_elem_count - i - 1] << std::endl;
    }
    free(stack_syms);

    std::cerr << "------------------------------------------------------------"
              << std::endl;

    exit(1);
}



int main(int argc, char** argv)
{
    std::set_terminate(terminate_handler);

    if (argc < 2)
    {
        std::cerr << "ERROR: Must provide path to patch config file" << std::endl;
        return 1;
    }

    auto patchConfig = PatchConfig::load(argv[1]);
    std::cout << "Setting up patch " << patchConfig.getName() << std::endl;

    Synth synth;
    synth.reset();



    for (uint16_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        double noteBaseFrequency = 440.0;
        // if (voiceNum == 2)
        // {
        //     noteBaseFrequency = 150.0;
        // }


        auto algorithmNumber = patchConfig.getAlgorithm();
        synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_ALGORITHM, algorithmNumber - 1);
        // synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_AMPLITUDE_ADJUST, toFixed(1.0 / patchConfig.getNumCarriers()));
        // synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_AMPLITUDE_ADJUST, toFixed(1.0));

        synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_KEYON, false);

        for (uint16_t opNum = 0; opNum < 8; opNum++)
        {
            auto opConfig = patchConfig.getOperatorConfig(opNum);

            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ATTACK_LEVEL, opConfig.getAttackLevel());
            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_SUSTAIN_LEVEL, opConfig.getSustainLevel());
            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ATTACK_RATE, opConfig.getAttackRate());
            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_DECAY_RATE, opConfig.getDecayRate());
            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_RELEASE_RATE, opConfig.getReleaseRate());

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
            printf("phaseStep = %f \n", opConfig.getFrequencyRatio());
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_PHASE_STEP_HIGH, phaseStep >> 8);
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_PHASE_STEP_LOW, phaseStep & 0xff);
        }

        if (voiceNum == 1 || voiceNum == 2)
        {
            synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_KEYON, true);
        }
    }

    // TODO: Graphics?
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        std::cerr << "Failed to init SDL" << std::endl;
        return 1;
    }


    double seconds = 3.0;
    auto& rBuffer = synth.getSampleBuffer();
    while (rBuffer.size() < static_cast<uint32_t>(SAMPLE_FREQUENCY * seconds))
    {
        synth.tick();
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


    SDL_PauseAudioDevice(device, 0);

    // TODO
    SDL_Delay(static_cast<uint32_t>(seconds * 1000));

    SDL_CloseAudio();
    SDL_Quit();

    return 0;
}
