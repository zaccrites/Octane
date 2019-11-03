
#pragma once

#include <stdint.h>

// See section 39 of the reference manual (RM0090).


// TODO: Make a cpp file for this?


// Can this be a totally static file? Or just a collection of functions
// in a namespace?

class DeviceId
{
public:

    static constexpr uint32_t ADDRESS = 0x1fff7a10;
    static constexpr uint32_t LENGTH = 12;

    static DeviceId get()
    {
        return {};
    }

    /// Read device ID bytes into a buffer
    void get_bytes(uint8_t* pBuffer) const
    {
        pBuffer[0] = static_cast<uint8_t>(m_LotNumber >> 0);
        pBuffer[1] = static_cast<uint8_t>(m_LotNumber >> 8);
        pBuffer[2] = static_cast<uint8_t>(m_LotNumber >> 16);
        pBuffer[3] = static_cast<uint8_t>(m_LotNumber >> 24);
        pBuffer[4] = static_cast<uint8_t>(m_WaferNumber >> 0);
        pBuffer[5] = static_cast<uint8_t>(m_WaferNumber >> 8);
        pBuffer[6] = static_cast<uint8_t>(m_WaferNumber >> 16);
        pBuffer[7] = static_cast<uint8_t>(m_WaferNumber >> 24);
        pBuffer[8] = static_cast<uint8_t>(m_WaferCoords >> 0);
        pBuffer[9] = static_cast<uint8_t>(m_WaferCoords >> 8);
        pBuffer[10] = static_cast<uint8_t>(m_WaferCoords >> 16);
        pBuffer[11] = static_cast<uint8_t>(m_WaferCoords >> 24);
    }

    uint32_t length() const
    {
        return LENGTH;
    }

    uint32_t get_wafer_coords() const
    {
        return m_WaferCoords;
    }

    uint32_t get_wafer_number() const
    {
        return m_WaferNumber;
    }

    uint32_t get_lot_number() const
    {
        return m_LotNumber;
    }

private:

    DeviceId() :
        m_WaferCoords { *reinterpret_cast<uint32_t*>(ADDRESS + 0x00) },
        m_WaferNumber { *reinterpret_cast<uint32_t*>(ADDRESS + 0x04) },
        m_LotNumber { *reinterpret_cast<uint32_t*>(ADDRESS + 0x08) }
    {
    }

    const uint32_t m_WaferCoords;
    const uint32_t m_WaferNumber;
    const uint32_t m_LotNumber;
};


