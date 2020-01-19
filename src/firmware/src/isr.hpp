
#include "Spi.hpp"


#ifndef ISR_HPP
#define ISR_HPP

namespace octane::isr
{


void registerSpiHandler(SPI_TypeDef* pRawSpi, octane::Spi* pSpi);


}

#endif
