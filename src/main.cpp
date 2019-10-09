
#include <printf.h>
#include <iostream>

#include <SDL2/SDL.h>

#include "synth.hpp"
#include "server.hpp"

#include <thread>
#include <chrono>
#include <atomic>



const uint32_t SAMPLE_FREQUENCY = 44100;


int main()
{
    Synth synth;
    synth.reset();

    Server server;
    std::atomic<bool> serverRunning { true };
    std::thread serverThread { [&server, &serverRunning]() {
        server.start();
        printf("Server over \n");
        serverRunning = false;
    } };


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


    SDL_PauseAudioDevice(device, 0);

    // SDL_Delay(seconds * 1000 + 500);
    // while (true)
    // for (int t = 0; t < seconds * 1000; t += SLEEP_PERIOD)
    const uint32_t SLEEP_PERIOD = 10;

    // FILE* csv = fopen("data.csv", "w");
    while (serverRunning)
    {
        Command command;
        while (server.getCommand(command))
        {
            printf("Setting register %04x to %04x \n", command.registerNumber, command.registerValue);
            synth.writeRegister(command.registerNumber, command.registerValue);
        }
        // printf("checked for commands \n");

        // Should really time above and delay by extra time
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_PERIOD));
        // SDL_Delay(10);
    }
    serverThread.join();
    // fclose(csv);

    SDL_CloseAudio();
    SDL_Quit();

    // server.end();
    return 0;

}
