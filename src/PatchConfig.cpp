
#include <cassert>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "PatchConfig.hpp"


// NOTE: This version loads via JSON, but the microcontroller
// will load via a binary version stored to an EEPROM.


// TODO: Better error handling and input validation.
// Silently using the default on error is a bad idea.
class RawOperatorConfig
{
public:

    RawOperatorConfig(json& rawJson) : m_Json(rawJson)
    {
    }

    OperatorConfig::Waveform getWaveform()
    {
        auto option = m_Json["waveform"];
        if (option == "square")
        {
            return OperatorConfig::Waveform::Square;
        }
        else if (option == "sine")
        {
            return OperatorConfig::Waveform::Sine;
        }
        else
        {
            std::cerr << "unknown waveform" << std::endl;
            return OperatorConfig::Waveform::Sine;
        }
    }

    double getFrequencyRatio()
    {
        auto option = m_Json["frequency_ratio"];
        if (option.is_number())
        {
            return option.get<double>();
        }
        else
        {
            std::cerr << "freq ratio error" << std::endl;
            return 1.0;
        }
    }

    uint16_t getAttackLevel() { return getAdsrValue("attack_level", 255); }
    uint16_t getSustainLevel() { return getAdsrValue("sustain_level", 255); }
    uint16_t getAttackRate() { return getAdsrValue("attack_rate", 255); }
    uint16_t getDecayRate() { return getAdsrValue("decay_rate", 255); }
    uint16_t getReleaseRate() { return getAdsrValue("release_rate", 255); }


private:

    uint16_t getAdsrValue(const char* key, uint8_t defaultValue)
    {
        auto option = m_Json[key];
        uint8_t rawValue = option.is_number() ? option.get<uint8_t>() : defaultValue;
        // Convert unsigned 8 bit to "signed" 7 bit
        return rawValue << 7;
    }


private:
    json m_Json;


};

class RawPatchConfig
{
public:

    RawPatchConfig(const char* path) : m_Json {}
    {
        std::ifstream jsonFile { path };
        jsonFile >> m_Json;
    }

    uint16_t getAlgorithm()
    {
        uint16_t result;
        auto option = m_Json["algorithm"];
        if (option.is_number())
        {
            result = option.get<uint16_t>();
            if (result < 1) result = 1;
            if (result > 32) result = 32;
        }
        else
        {
            result = 1;
            std::cerr << "Failed to read algorithm number, defaulting to " << result << std::endl;
        }
        return result;
    }

    std::string getName()
    {
        auto option = m_Json["name"];
        if (option.is_string())
        {
            return option.get<std::string>();
        }
        else
        {
            return "<ERROR>";
        }
    }

    RawOperatorConfig getOp(int opNum)
    {
        assert(0 <= opNum && opNum < 6);
        return RawOperatorConfig { m_Json["operators"][opNum] };
    }


private:
    json m_Json;

};



PatchConfig PatchConfig::load(const char* path)
{
    RawPatchConfig rawConfig { path };

    PatchConfig config;
    config.m_Algorithm = rawConfig.getAlgorithm();
    config.m_Name = rawConfig.getName();

    for (int opNum = 0; opNum < 6; opNum++)
    {
        auto rawOpConfig = rawConfig.getOp(opNum);
        auto& opConfig = config.getOperatorConfig(opNum);

        opConfig.m_Waveform = rawOpConfig.getWaveform();
        opConfig.m_FrequencyRatio = rawOpConfig.getFrequencyRatio();
        opConfig.m_AttackLevel = rawOpConfig.getAttackLevel();
        opConfig.m_SustainLevel = rawOpConfig.getSustainLevel();
        opConfig.m_AttackRate = rawOpConfig.getAttackRate();
        opConfig.m_DecayRate = rawOpConfig.getDecayRate();
        opConfig.m_ReleaseRate = rawOpConfig.getReleaseRate();
    }

    return config;
}





// TODO: Zero-based index these too
uint16_t PatchConfig::getNumCarriers() const
{
    switch (m_Algorithm)
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
            // Impossible until further algorithms are defined.
            assert(false);
    }
}
