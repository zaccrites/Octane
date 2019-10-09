
#include <printf.h>
#include <iostream>

#include <SDL2/SDL.h>

#include "synth.hpp"
#include "server.hpp"

#include <thread>
#include <chrono>
#include <atomic>



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


int main()
{
    Synth synth;
    synth.reset();

    Server server;
    std::atomic<bool> serverRunning { true };
    std::thread serverThread { [&server, &serverRunning]() {
        printf("Server start\n");
        // server.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(20000));
        printf("Server over \n");
        serverRunning = false;
    } };


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

            if (voiceNum == 1  || true)
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


    auto& rBuffer = synth.getSampleBuffer();
    while (rBuffer.size() < SAMPLE_FREQUENCY * 10)
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

    while (serverRunning)
    {
        Command command;
        while (server.getCommand(command))
        {
            printf("Setting register %04x to %04x \n", command.registerNumber, command.registerValue);
            synth.writeRegister(command.registerNumber, command.registerValue);
        }

        // Should really time above and delay by extra time
        // std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_PERIOD));
        SDL_Delay(10);
    }
    serverThread.join();

    SDL_CloseAudio();
    SDL_Quit();

    return 0;

}
