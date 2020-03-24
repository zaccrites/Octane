
#include <iostream>

#include <SDL2/SDL.h>

#include "Synth.hpp"



const uint32_t SAMPLE_FREQUENCY = 44100;




int main(int argc, char** argv)
{

    std::cout << "Hello World!" << std::endl;



    Synth synth;
    synth.reset();

    // TODO: remove this function, move to shared
    // OR, generalize the interface to the FPGA itself
    synth.populateSineTable();



    const bool playAudio = true;





    for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum++)
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
            // noteBaseFrequency = 523.251;  // C5
        }
        // noteBaseFrequency = 100.0 * (1 + voiceNum);
        // noteBaseFrequency = 440.0;


        // auto makeAlgorithmWord = [](uint8_t modulation, bool isCarrier, uint8_t numCarriers)
        uint16_t algorithmWords[8] = {
            //       7654321
            //xxxxxx mmmmmmm nnn c
            0b000000'0000000'000'0,  // OP1
            0b000000'0000000'000'1,  // OP2
            0b000000'0000000'000'0,  // OP3
            0b000000'0000000'000'0,  // OP4
            0b000000'0000000'000'0,  // OP5
            0b000000'0000000'000'0,  // OP6
            0b000000'0000000'000'0,  // OP7
            0b000000'0000000'000'0,  // OP8
        };

        synth.setNoteOn(voiceNum, false);

        for (uint8_t opNum = 0; opNum < 8; opNum++)
        {
            // auto opConfig = patchConfig.getOperatorConfig(opNum);

            // TODO: Use JSON
            // uint8_t feedbackLevel = (opNum == 0) ? 255 : 0;
            uint8_t feedbackLevel = 128;
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_FEEDBACK_LEVEL, feedbackLevel);

            // TODO: Use JSON
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ALGORITHM, algorithmWords[opNum]);

            // uint16_t phaseStep = phaseStepForFrequency(noteBaseFrequency * opConfig.getFrequencyRatio());
            uint16_t phaseStep = 1398;
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_PHASE_STEP, phaseStep);

            bool isCarrier = algorithmWords[opNum] & 0x0001;


            // TODO: Clean this all up. Put into a Patch class that can
            // do all these calculations and output real values to pass
            // to the microcontroller after loading the binary configuration.

            auto modulationIndexForLevel = [](uint16_t level) -> double {
                // Vary linearly up to I=15
                return level * 15.0 / 1000.0;
            };

            auto carrierAmplitudeForLevel = [](uint16_t level) -> double {
                // 1000 is 100% intensity, but 500 is half as loud (10% intensity).
                // 250 is a quarter as loud (1% intensity), etc.
                const double exponent = std::log(10) / std::log(2);
                return std::pow(level, exponent) / std::pow(1000.0, exponent);
            };


            auto fixOperatorLevel = [carrierAmplitudeForLevel, modulationIndexForLevel, phaseStep, isCarrier](uint16_t level) -> uint16_t {
                if (isCarrier)
                {
                    double multiplier = carrierAmplitudeForLevel(level);
                    // printf("multiplier for level %d is %f \n", level, multiplier);
                    return static_cast<uint16_t>(0x3fff * multiplier);
                }
                else
                {
                    double modulationIndex = modulationIndexForLevel(level);
                    return static_cast<uint16_t>(modulationIndex * phaseStep);
                }
            };
            auto fixOperatorRate = [carrierAmplitudeForLevel, modulationIndexForLevel, phaseStep, isCarrier](uint16_t rate) -> uint16_t {

                if (isCarrier)
                {
                    double multiplier = carrierAmplitudeForLevel(rate);
                    return static_cast<uint16_t>(0x0fff * multiplier);
                }
                else
                {
                    double modulationIndex = modulationIndexForLevel(rate);
                    return static_cast<uint16_t>(modulationIndex * phaseStep) / 4;
                }
            };

            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_ATTACK_LEVEL, fixOperatorLevel(opConfig.getAttackLevel()));
            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_SUSTAIN_LEVEL, fixOperatorLevel(opConfig.getSustainLevel()));
            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_ATTACK_RATE, fixOperatorRate(opConfig.getAttackRate()));
            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_DECAY_RATE, fixOperatorRate(opConfig.getDecayRate()));
            // synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_RELEASE_RATE, fixOperatorRate(opConfig.getReleaseRate()));

            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_ATTACK_LEVEL, 0xffff);
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_SUSTAIN_LEVEL, 0xffff);
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_ATTACK_RATE, 0xffff);
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_DECAY_RATE, 0xffff);
            synth.writeOperatorRegister(voiceNum, opNum, Synth::OP_PARAM_ENVELOPE_RELEASE_RATE, 0xffff);

        }
    }

    for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        if (voiceNum == 0)
        {
            synth.setNoteOn(voiceNum, true);
        }
    }

    // Add some overhead for setting up the sine table
    double seconds = 0.225 + (playAudio ? 1.0 : 0.3);
    // seconds *= 10;

    // double seconds = 0.005;
    // double seconds = 0.225 + (playAudio ? 1.0 : 0.3);
    auto& rBuffer = synth.getSampleBuffer();

    // double noteOn = false;
    const size_t samplesNeeded { static_cast<uint32_t>(SAMPLE_FREQUENCY * seconds) };
    double t_last = 0.0;
    bool ledOn = false;
    while (rBuffer.size() < samplesNeeded)
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


        // TODO: fix
        const double period = 0.01;
        if (std::fmod(t_last, period) < (period / 2) && std::fmod(t, period) > (period / 2))
        {
            ledOn = ! ledOn;
            synth.writeOperatorRegister(0, 0, Synth::PARAM_LED_CONFIG, ledOn ? 0xffff : 0xff0f);
        }


        synth.spiSendReceive();

        // printf("rBuffer.size() = %d \n", rBuffer.size());

        t_last = t;
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
