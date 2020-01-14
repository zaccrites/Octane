
#pragma once

#include <cstdint>


// TODO: This is a good candidate to extract as a platform interface

namespace octane
{



class DeviceId
{
public:
    static DeviceId get();

    std::uint32_t getPart1() const { return m_IdParts[0]; }
    std::uint32_t getPart2() const { return m_IdParts[1]; }
    std::uint32_t getPart3() const { return m_IdParts[2]; }

private:
    DeviceId(std::uint32_t part1, std::uint32_t part2, std::uint32_t part3);
    const std::uint32_t m_IdParts[3];

};


}
