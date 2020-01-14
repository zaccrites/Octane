
#include <stm32f4xx.h>

#include "DeviceId.hpp"


namespace octane
{


DeviceId DeviceId::get()
{
    const auto* pUidBase = reinterpret_cast<std::uint32_t*>(UID_BASE);
    return {pUidBase[0], pUidBase[1], pUidBase[2]};
}

DeviceId::DeviceId(std::uint32_t part1, std::uint32_t part2, std::uint32_t part3) :
    m_IdParts { part1, part2, part3 }
{
}


}
