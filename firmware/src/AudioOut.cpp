
#include "AudioOut.hpp"

#include <stm32f4xx.h>


namespace octane
{

AudioOut AudioOut::instance;


AudioOut::AudioOut()
{
}


AudioOut& AudioOut::getInstance()
{
    return AudioOut::instance;
}


void AudioOut::init()
{
    // https://git.morgothdisk.com/C-CPP/MULTIPLATFORM-C-SDK/blob/3eea565a265dc36dda09fcc141a40be38001454e/SDK/lib/device/cs43l22.c

    // See section 4.9 of CS43L22 datasheet

    GPIOD->BSRR = GPIO_BSRR_BS4;  // Bring /RESET high (4.9.2)

    // Power down (4.9.3)
    writeRegister(REG_POWER_CTL1, 0x01);


    // "required" initialization writes (4.9.4 and 4.11)
    writeRegister(0x00, 0x99);
    writeRegister(0x47, 0x80);
    writeRegister(0x32, 0x40);
    writeRegister(0x32, 0x00);
    writeRegister(0x00, 0x00);





}



// void AudioOut::play()
// {

//     if (m_Stopped)
//     {
//         // TODO: REG_MISC_CTRL: enable digital soft ramp

//         setMute(false);



//         // Power on
//         writeRegister(REG_POWER_CTL1, 0x9e);
//     }


// }


// void AudioOut::stop()
// {
//     // See datasheet section 4.10

//     setMute(true);
//     writeRegister(REG_POWER_CTL1, 0x9f);

//     m_Stopped = true;
// }


void AudioOut::setMute(bool mute)
{

}


// TODO: Add more options, like duration and pitch
void AudioOut::beep()
{
    // For now, just a single short beep

    // [7.15]
    uint8_t value =
        (0b0011 << 4) |  // Frequency = 666.67 Hz (E5)
        (0b0010 << 0);   // ON time = 780 ms
    writeRegister(REG_BEEP_FREQ_AND_ON_TIME, value);

    // [7.16]
    value =
        (0b000   << 5) |  // OFF time = 1.23 s
        (0b00000 << 0);   // Volume = -6 dB
    writeRegister(REG_BEEP_VOLUME_AND_OFF_TIME, value);

    // [7.17]
    value = readRegister(REG_BEEP_AND_TONE_CONFIG);
    value &= ~0b11100000;
    value |=
        (0b01 << 6) |  // single beep
        (0 << 5);      // mix beep with digital audio input
    writeRegister(REG_BEEP_AND_TONE_CONFIG, value);


}



void AudioOut::beginByteTransfer()
{
    // TODO: Error handling?
    // Single byte transfer

    // [RM0090 27.3.3]

    // "Start condition"
    I2C1->CR1 |= I2C_CR1_START;  // ???
    (void)I2C1->SR1;

    // "Slave address transmission"
    I2C1->DR = (I2C_SLAVE_ADDRESS << 1) | 0;  // transmit
    (void)I2C1->SR1;
    (void)I2C1->SR2;



    // Sharing this logic will be hard because of the way I have
    // to talk to the CS43L22

    /*

    Write:
    -----------------------------------------------
    START
    slave address (write)
    ACK
    register number (optional increment)
    ACK
    DATA ACK
        (DATA, ACK)* if more data to write
    STOP


    Read:
    -----------------------------------------------
    START
    slave address (write)
    ACK
    register number (optional increment)
    ACK
    STOP  ("aborted write")
    START
    slave address (read)
    ACK
    DATA
        (ACK, DATA)* if more data to read
    NACK
    STOP

    */


}


void AudioOut::writeRegister(uint8_t regNum, uint8_t value)
{
    // TODO: Write all registers using auto increment?

    // TODO: Use interrupts to handle this? DMA?


    // [RM0090 27.3.3]

    // "Start condition"
    I2C1->CR1 |= I2C_CR1_START;  // ???
    (void)I2C1->SR1;

    // "Slave address transmission"
    I2C1->DR = (I2C_SLAVE_ADDRESS << 1) | 0;  // WRITE
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    while ( ! (I2C1->SR1 & I2C_SR1_ADDR));  // wait

    // Send MAP (register number) byte
    // NOTE: INCR bit explicitly not set
    I2C1->DR = regNum;
    while ( ! (I2C1->SR1 & I2C_SR1_TXE));  // wait

    // Send data byte
    I2C1->DR = value;
    while ( ! (I2C1->SR1 & I2C_SR1_TXE));  // wait
    I2C1->CR1 |= I2C_CR1_STOP;




    // // TODO: Look at e.g. Figure 243 for EVx events

    // beginByteTransfer();

    // // "Master transmitter"
    // I2C1->DR = value;
    // // wait?

    // // "Closing the communication"
    // I2C1->CR1 |= I2C_CR1_STOP;

    // // while ( ! (I2C1->SR & I2C_SR_BTF));  // wait for transfer to finish

    // while ( ! (I2C1->SR1 & I2C_SR1_TxE));  // wait for transmit to finish

}


uint8_t AudioOut::readRegister(uint8_t regNum)
{
    // TODO: Read all registers using auto increment?

    // TODO: Use interrupts to handle this? DMA?


    // [RM0090 27.3.3]

    // "Start condition"
    I2C1->CR1 |= I2C_CR1_START;  // ???
    (void)I2C1->SR1;

    // "Slave address transmission"
    I2C1->DR = (I2C_SLAVE_ADDRESS << 1) | 0;  // WRITE
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    while ( ! (I2C1->SR1 & I2C_SR1_ADDR));  // wait

    // Send MAP (register number) byte
    // NOTE: INCR bit explicitly not set
    I2C1->DR = regNum;
    while ( ! (I2C1->SR1 & I2C_SR1_TXE));  // wait

    // "Abort" write and start a new cycle
    I2C1->CR1 |= I2C_CR1_STOP | I2C_CR1_START;
    (void)I2C1->SR1;

    // "Slave address transmission"
    I2C1->DR = (I2C_SLAVE_ADDRESS << 1) | 0;  // READ
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->CR1 &= ~I2C_CR1_ACK;  // we only want one byte for now
    while ( ! (I2C1->SR1 & I2C_SR1_ADDR));  // wait
    while ( ! (I2C1->SR1 & I2C_SR1_RXNE));  // wait
    uint8_t value = I2C1->DR;
    I2C1->CR1 |= I2C_CR1_STOP;

    return value;


    // beginByteTransfer();
    // I2C1->ACK &= ~I2C_CR1_ACK;
    // while ( ! (I2C1->SR1 & I2C_SR1_ADDR));  // wait
    // I2C1->CR |= I2C_CR1_STOP;


    // // "Master receiver"
    // while ( ! (I2C1->SR1 & I2C_SR_RxNE));  // wait



    // // while ( ! (I2C1->SR & I2C_SR_BTF));  // wait for transfer to finish

    // return I2C1->DR;
}



}
