
#include <printf.h>
#include <iostream>
#include <fstream>

#include <SDL2/SDL.h>

#include "synth.hpp"
// #include "server.hpp"

#include <thread>
#include <chrono>
#include <atomic>


// TODO: Extract JSON handling to its own file to avoid having to build this
// over and over. Perhaps create a PatchConfig and OperatorConfig class.
#include <nlohmann/json.hpp>



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


volatile size_t req = 0;  // TODO: Remove

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

    // Server server;
    // std::atomic<bool> serverRunning { true };
    // std::thread serverThread { [&server, &serverRunning]() {
    //     printf("Server start\n");
    //     // server.start();
    //     std::this_thread::sleep_for(std::chrono::milliseconds(20000));
    //     printf("Server over \n");
    //     serverRunning = false;
    // } };


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


    // TODO:
    //  - Fixed operator frequency
    //  - Pitch wheel
    //  - Pitch envelope
    //  - Transposition, feedback, etc.
    //  - LFO
    //  - Keyboard splitting/scaling
    //  - More advanced ADSR envelope shape

    std::ifstream patchConfigFile { "patches/test1.json" };
    nlohmann::json patchConfig;
    patchConfigFile >> patchConfig;

    // printf("Setting up patch \"%s\" \n", patchConfig["name"]);
    std::cout << "Setting up patch " << patchConfig["name"] << std::endl;

    // TODO: Input validation




    double noteBaseFrequency = 440.0;

    // Set phase step
    for (uint16_t operatorNum = 1; operatorNum <= 6; operatorNum++)
    {
        auto operatorConfig = patchConfig["operators"][operatorNum - 1];

        for (uint16_t voiceNum = 1; voiceNum <= 16; voiceNum++)
        {
            // TODO: Input validation. If a key is misspelled then you get
            // a C++ exception about how "type must be number, but is null"
            auto attackLevel = operatorConfig["attack_level"].get<uint8_t>();
            auto sustainLevel = operatorConfig["sustain_level"].get<uint8_t>();
            auto attackRate = operatorConfig["attack_rate"].get<uint8_t>();
            auto decayRate = operatorConfig["decay_rate"].get<uint8_t>();
            auto releaseRate = operatorConfig["release_rate"].get<uint8_t>();
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_ATTACK_LEVEL, attackLevel << 7);
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_SUSTAIN_LEVEL, sustainLevel << 7);
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_ATTACK_RATE, attackRate << 7);
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_DECAY_RATE, decayRate << 7);
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_RELEASE_RATE, releaseRate << 7);



            auto frequencyRatio = operatorConfig["frequency_ratio"].get<double>();
            uint16_t phaseStep = phaseStepForFrequency(noteBaseFrequency * frequencyRatio);
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_PHASE_STEP, phaseStep);

            uint16_t waveform;
            if (operatorConfig["waveform"] == "sine")
            {
                waveform = Synth::OP_WAVEFORM_SINE;
            }
            else
            {
                waveform = Synth::OP_WAVEFORM_SQUARE;
            }
            synth.writeOperatorRegister(voiceNum, operatorNum, Synth::OP_PARAM_WAVEFORM, waveform);


            // case 5:
            //     phaseStep = phaseStepForFrequency(220.0);
            //     setEnvelope(
            //         1.0,  // attack level
            //         0.2,  // sustain level
            //         0.001,  // attack rate
            //         0.05,  // decay rate
            //         0.005   // release rate
            //     );
            //     break;

            // case 6:
            //     phaseStep = phaseStepForFrequency(440.0);
            //     setEnvelope(
            //         1.0,  // attack level
            //         0.7,  // sustain level
            //         0.05,  // attack rate
            //         0.0005,  // decay rate
            //         0.005   // release rate
            //     );
            //     break;


            auto algorithmNumber = patchConfig["algorithm"].get<uint16_t>();
            synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_ALGORITHM, algorithmNumber - 1);
            synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_AMPLITUDE_ADJUST, toFixed(1.0 / carriersForAlgorithm(algorithmNumber)));
            // synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_AMPLITUDE_ADJUST, toFixed(1.0));


            if (voiceNum == 1)
            {
                synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_KEYON, true);
            }
            else
            {
                synth.writeVoiceRegister(voiceNum, Synth::VOICE_PARAM_KEYON, false);
            }

        }


    }



    // TODO: Graphics?
    // if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        std::cerr << "Failed to init SDL" << std::endl;
        return 1;
    }


    uint32_t seconds = 3;
    auto& rBuffer = synth.getSampleBuffer();
    while (rBuffer.size() < SAMPLE_FREQUENCY * seconds)
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

    // SDL_Delay(seconds * 1000 + 500);
    // while (true)
    // for (int t = 0; t < seconds * 1000; t += SLEEP_PERIOD)
    const uint32_t SLEEP_PERIOD = 10;

    // while (serverRunning)
    // {
    //     Command command;
    //     while (server.getCommand(command))
    //     {
    //         printf("Setting register %04x to %04x \n", command.registerNumber, command.registerValue);
    //         synth.writeRegister(command.registerNumber, command.registerValue);
    //     }

    //     // Should really time above and delay by extra time
    //     // std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_PERIOD));
    //     SDL_Delay(10);
    // }
    // serverThread.join();

    // TODO
    SDL_Delay(seconds * 1000);

    SDL_CloseAudio();
    SDL_Quit();

    return 0;

}
