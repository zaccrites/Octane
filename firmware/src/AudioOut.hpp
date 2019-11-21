
#ifndef AUDIO_OUT_HPP
#define AUDIO_OUT_HPP

#include <stdint.h>

namespace octane
{


/// Manages the interface to the audio output device, the CS43L22
class AudioOut
{
public:

    static AudioOut& getInstance();

    void init();

    void setMute(bool mute);

    void beep();


    // void stop();
    // void play();



private:

    static constexpr uint8_t I2C_SLAVE_ADDRESS { (0b100101 << 1) | 0 };


    static constexpr uint8_t REG_CHIPID { 0x01 };
    static constexpr uint8_t REG_POWER_CTL1 { 0x02 };
    // static constexpr uint8_t POWER_CONTROL_2_ADDR { 0x04 };
    // static constexpr uint8_t CLOCKING_CONTROL { 0x05 };
    // static constexpr uint8_t INTERFACE_CONTROL_1 { 0x06 };
    // static constexpr uint8_t INTERFACE_CONTROL_2 { 0x07 };
    // static constexpr uint8_t PASSTHOUGH_SELECT_A { 0x08 };
    // static constexpr uint8_t PASSTHOUGH_SELECT_B { 0x09 };
    // static constexpr uint8_t ANALOG_ZC_AND_SR { 0x0a };
    // static constexpr uint8_t PASSTHROUGH_GANG_CONTROL { 0x0c };
    // static constexpr uint8_t PLAYBACK_CONTROL_1 { 0x0d };


    static constexpr uint8_t REG_BEEP_FREQ_AND_ON_TIME { 0x1c };
    static constexpr uint8_t REG_BEEP_VOLUME_AND_OFF_TIME { 0x1d };
    static constexpr uint8_t REG_BEEP_AND_TONE_CONFIG { 0x1e };
    // static constexpr uint8_t


private:

    AudioOut();
    static AudioOut instance;





    void writeRegister(uint8_t regNum, uint8_t value);
    uint8_t readRegister(uint8_t regNum);
    void beginByteTransfer();


/*

PD4 - N/A - RESET

PB6 - I2C1_SCL - SCL
PB9 - I2C1_SDA - SDA

PA4 - I2S3_WS - LRCK/AIN1x
PC7 - I2S3_MCK - MCLK
PC10 - I2S3_CK - SCLK
PC12 - I2S3_SD - SDIN



*/



private:



};


}


#endif
