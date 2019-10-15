
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


class OperatorConfig
{
friend class PatchConfig;
public:

    enum class Waveform { Sine, Square };

    Waveform getWaveform() const { return m_Waveform; }
    double getFrequencyRatio() const { return m_FrequencyRatio; }

    uint8_t getAttackLevel() const { return m_AttackLevel; }
    uint8_t getSustainLevel() const { return m_SustainLevel; }
    uint8_t getAttackRate() const { return m_AttackRate; }
    uint8_t getDecayRate() const { return m_DecayRate; }
    uint8_t getReleaseRate() const { return m_ReleaseRate; }

private:
    Waveform m_Waveform;
    double m_FrequencyRatio;  // TODO: Use fixed point instead
    // What about coarse vs fine vs detune? I imagine these are just abstractions over an octave/step/fine-grained percentage change of frequency.

    uint8_t m_AttackLevel;
    uint8_t m_SustainLevel;
    uint8_t m_AttackRate;
    uint8_t m_DecayRate;
    uint8_t m_ReleaseRate;

};


class PatchConfig
{
public:

    static PatchConfig load(const char* path);
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
