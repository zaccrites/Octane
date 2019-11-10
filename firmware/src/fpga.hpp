
#ifndef FPGA_HPP
#define FPGA_HPP

#include <stdint.h>


namespace octane::fpga
{


const uint8_t OP_PARAM_PHASE_STEP  { 0x00 };
const uint8_t OP_PARAM_ALGORITHM      { 0x01 };

const uint8_t OP_PARAM_ENVELOPE_ATTACK_LEVEL   { 0x02 };
const uint8_t OP_PARAM_ENVELOPE_SUSTAIN_LEVEL  { 0x03 };
const uint8_t OP_PARAM_ENVELOPE_ATTACK_RATE    { 0x04 };
const uint8_t OP_PARAM_ENVELOPE_DECAY_RATE     { 0x05 };
const uint8_t OP_PARAM_ENVELOPE_RELEASE_RATE   { 0x06 };

const uint8_t OP_PARAM_FEEDBACK_LEVEL   { 0x07 };


const uint8_t PARAM_NOTEON_BANK0  { 0x10 };
const uint8_t PARAM_NOTEON_BANK1  { 0x11 };
const uint8_t PARAM_LED_CONFIG  { 0x12 };



void reset();

void init();

void writeRegister(uint16_t registerNumber, uint16_t value);

void writeOperatorRegister(uint8_t voiceNum, uint8_t operatorNum, uint8_t parameter, uint16_t value);


void clearNotesOn();

void setNoteOn(uint8_t voiceNum, bool noteOn);



}

#endif
