
#include "init.hpp"
#include <printf.h>

#include "fpga.hpp"


#include <stdint.h>
#include <stm32f4xx.h>




extern volatile bool fpgaLedOn;


extern volatile bool newSampleAvailable;
extern volatile uint16_t currentSample;



// TODO: Generate sine table at boot instead
#define SINE_TABLE_IMPLEMENTATION
#include "sine_table.h"

// TODO
// #include <array>
void initFpga(octane::Fpga& rFpga)
{
    // Populate sine table memory
    for (uint16_t i = 0; i < SINE_TABLE_LENGTH; i++)
    {
        // TODO: Figure out how to not have to commitRegisterWrites
        // after every register write


        const uint16_t registerNumber = (0b11 << 14) | i;
        rFpga.writeRegister(registerNumber, SINE_TABLE[i]);
        rFpga.commitRegisterWrites();
        // printf("Wrote 0x%04x to sine table slot %d \r\n", SINE_TABLE[i], i);
        while (rFpga.getSpiTransferInProgress());  // wait

        // if (i % 256 == 0)
        // {
        //     printf("Committing register writes \r\n");
        //     rFpga.commitRegisterWrites();
        //     while (rFpga.getSpiTransferInProgress());  // wait
        // }
    }
    // rFpga.commitRegisterWrites();
    // while (rFpga.getSpiTransferInProgress());  // wait


    const uint16_t algorithmWords[8] = {
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

    rFpga.clearNotesOn();
    rFpga.commitRegisterWrites();
    while (rFpga.getSpiTransferInProgress());  // wait


    for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        for (uint8_t opNum = 0; opNum < 8; opNum++)
        {

            // uint8_t feedbackLevel = (opNum == 0) ? 255 : 0;
            uint8_t feedbackLevel = 0;
            rFpga.writeOperatorRegister(voiceNum, opNum, octane::Fpga::OP_PARAM_FEEDBACK_LEVEL, feedbackLevel);
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait

            rFpga.writeOperatorRegister(voiceNum, opNum, octane::Fpga::OP_PARAM_ALGORITHM, algorithmWords[opNum]);
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait

            uint16_t phaseStep = 1398;  // 1 kHz tone at 12 MHz / 256 = 46.875 kHz sample freq
            // uint16_t phaseStep = 100;  // 1 kHz tone at 12 MHz / 256 = 46.875 kHz sample freq
            // uint16_t phaseStep = 500;  // 1 kHz tone at 12 MHz / 256 = 46.875 kHz sample freq
            rFpga.writeOperatorRegister(voiceNum, opNum, octane::Fpga::OP_PARAM_PHASE_STEP, phaseStep);
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait

            // uint16_t attackLevel = 1000;
            // uint16_t sustainLevel = 1000;
            // uint16_t attackRate = 1000;
            // uint16_t decayRate = 1000;
            // uint16_t releaseRate = 1000;

            rFpga.writeOperatorRegister(voiceNum, opNum, octane::Fpga::OP_PARAM_ENVELOPE_ATTACK_LEVEL, 0xffff); //fixOperatorLevel(attackLevel));
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait
            rFpga.writeOperatorRegister(voiceNum, opNum, octane::Fpga::OP_PARAM_ENVELOPE_SUSTAIN_LEVEL, 0xffff); //fixOperatorLevel(sustainLevel));
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait
            rFpga.writeOperatorRegister(voiceNum, opNum, octane::Fpga::OP_PARAM_ENVELOPE_ATTACK_RATE, 0xffff); // fixOperatorRate(attackRate));
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait
            rFpga.writeOperatorRegister(voiceNum, opNum, octane::Fpga::OP_PARAM_ENVELOPE_DECAY_RATE, 0xffff); // fixOperatorRate(decayRate));
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait
            rFpga.writeOperatorRegister(voiceNum, opNum, octane::Fpga::OP_PARAM_ENVELOPE_RELEASE_RATE, 0xffff); // fixOperatorRate(releaseRate));
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait
        }

        // rFpga.commitRegisterWrites();
        // while (rFpga.getSpiTransferInProgress());  // wait
    }

}




extern volatile bool updateSample;

void main()
{
    // GPIOD->BSRR = GPIO_BSRR_BS12;

    // octane::init();
    printf("\r\n======================== OCTANE ======================== \r\n\r\n");


    SPI2->CR1 |= SPI_CR1_SPE;    // enable SPI before comms



    printf("Starting FPGA initialization \r\n");

    auto& rFpga = octane::Fpga::getInstance();
    initFpga(rFpga);

    for (uint8_t voiceNum = 0; voiceNum < 32; voiceNum++)
    {
        // TODO: Ensure that the sample does not overflow.
        // I'm just disabling one voice until I can get that working.
        // if (voiceNum != 31) continue;
        // if (voiceNum < 16)
        if (voiceNum < 2)
        rFpga.setNoteOn(voiceNum, true);
    }

    rFpga.commitRegisterWrites();
    while (rFpga.getSpiTransferInProgress());  // wait

    printf("Finished FPGA initialization \r\n");



    // int i = 0;
    bool fpgaLedOnLast = false;

    int counter = 0;
    while (true)
    {

        if (fpgaLedOn != fpgaLedOnLast)
        {
            fpgaLedOnLast = fpgaLedOn;

            GPIOD->BSRR = fpgaLedOn ? GPIO_BSRR_BS13 : GPIO_BSRR_BR13;


            rFpga.writeRegister(
                (0b10 << 14) | (0x12 << 8) | (0 << 5) | 0,
                fpgaLedOn ? 0xffff : 0xff0f
            );
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait


            uint16_t sample = octane::Fpga::getInstance().getLatestSample();
            printf("Current sample: 0x%04x  (%d) \r\n", sample, static_cast<int16_t>(sample));

        }

        if (updateSample)
        {
            // Send a dummy write to get the newest sample
            rFpga.writeRegister(0x0000, 0x0000);
            rFpga.commitRegisterWrites();
            while (rFpga.getSpiTransferInProgress());  // wait for transfer to complete
            DAC1->DHR12R2 = (rFpga.getLatestSample() >> 4) + 0x07ff;

            updateSample = false;
        }

    }

}
