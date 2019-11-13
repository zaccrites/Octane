
#include <stm32f4xx.h>

#include "CriticalSection.hpp"


namespace octane
{


CriticalSectionLock::CriticalSectionLock()
{
    // TODO: Read CPSR to determine if interrupts were enabled to begin
    // with. If not, then don't re-enable them.

    __disable_irq();
}


CriticalSectionLock::~CriticalSectionLock()
{
    __enable_irq();
}


}
