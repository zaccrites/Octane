
#ifndef PATCHCONFIG_HPP
#define PATCHCONFIG_HPP


// TODO:
//  - Fixed operator frequency
//  - Pitch wheel
//  - Pitch envelope
//  - Transposition, feedback, etc.
//  - LFO
//  - Keyboard splitting/scaling
//  - More advanced ADSR envelope shape

#include <string>
#include <string_view>


class OperatorConfig
{
friend class PatchConfig;
public:

    enum class Waveform { Sine, Square };

    Waveform getWaveform() const { return m_Waveform; }
    double getFrequencyRatio() const { return m_FrequencyRatio; }

    uint8_t getEnvelopeL1() const { return m_EnvelopeLevels[0]; }
    uint8_t getEnvelopeL2() const { return m_EnvelopeLevels[1]; }
    uint8_t getEnvelopeL3() const { return m_EnvelopeLevels[2]; }
    uint8_t getEnvelopeL4() const { return m_EnvelopeLevels[3]; }

    uint8_t getEnvelopeR1() const { return m_EnvelopeRates[0]; }
    uint8_t getEnvelopeR2() const { return m_EnvelopeRates[1]; }
    uint8_t getEnvelopeR3() const { return m_EnvelopeRates[2]; }
    uint8_t getEnvelopeR4() const { return m_EnvelopeRates[3]; }

private:
    Waveform m_Waveform;
    double m_FrequencyRatio;  // TODO: Use fixed point instead
    // What about coarse vs fine vs detune? I imagine these are just abstractions over an octave/step/fine-grained percentage change of frequency.

    uint8_t m_EnvelopeLevels[4];
    uint8_t m_EnvelopeRates[4];

};


class PatchConfig
{
public:

    static PatchConfig load(std::string_view path);
    // TODO: Save?

    inline const std::string& getName() const { return m_Name; }
    uint16_t getAlgorithm() const { return m_Algorithm; }

    OperatorConfig& getOperatorConfig(uint8_t opNum)
    {
        return m_OperatorConfigs[opNum % 8];
    }

    uint16_t getNumCarriers() const;


private:
    std::string m_Name;
    uint16_t m_Algorithm;

    OperatorConfig m_OperatorConfigs[8];


};


#endif
