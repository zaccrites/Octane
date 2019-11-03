
#pragma once

#include <stdint.h>
#include <stddef.h>

namespace stringio
{


/// Transforms arbitrary bytes into a hex-encoded string.
/// Assumes that pBuffer points to a buffer with at least
/// the capacity to store 2*dataLength+1 characters
/// (two each for each data byte, plus the null terminator).
void hexlify(size_t dataLength, const uint8_t* pData, char* pBuffer);






// TODO: vsnprintf?

// template<class ... args>
size_t snprintf(char* pBuffer, size_t bufSize, const char* format, ...);



}  // namespace stringio
