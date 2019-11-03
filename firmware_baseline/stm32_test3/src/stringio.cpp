

#include "stringio.hpp"

#include <stdarg.h>


// TODO: Other overloads for other integer types? Signed?
template<int base>
static size_t int_to_string(char* pBuffer, char* pBufferEnd, uint32_t value)
{
    // The base is templated so that for powers of 2 the division
    // for each digit can be optimized to a bitwise shift-right.

    // static_assert(base == 2 || base == 8 || base == 10 || base == 16, "unsupported base");
    static_assert(base == 2 || base == 8 || base == 10 || base == 16, "unsupported base");

    // TODO: What about %x vs %X ?
    const char* digitchars = "0123456789abcdef";

    size_t digitsWritten = 0;
    while (pBuffer + digitsWritten < pBufferEnd)
    {
        pBuffer[digitsWritten] = digitchars[value % base];
        value /= base;
        digitsWritten += 1;

        if (value == 0)
        {
            break;
        }
    }

    // This will put the digits in backwards,
    // so now we have to reverse them in place.
    for (size_t i = 0; i < digitsWritten / 2; ++i)
    {
        char tmp = pBuffer[i];
        pBuffer[i] = pBuffer[digitsWritten - i - 1];
        // pBuffer[i] = '?';
        pBuffer[digitsWritten - i - 1] = tmp;
    }

    // Since this is a utility function used only by the printf functions
    // we DON'T put a null terminator on the end.
    return digitsWritten;


}



namespace stringio
{






void hexlify(size_t dataLength, const uint8_t* pData, char* pBuffer)
{
    const char* hexchars = "0123456789abcdef";
    for (size_t i = 0; i < dataLength; ++i)
    {
        *pBuffer++ = hexchars[pData[i] >> 4];
        *pBuffer++ = hexchars[pData[i] & 0x0f];
    }
    *pBuffer = '\0';
}


size_t snprintf(char* pBuffer, size_t bufSize, const char* format, ...)
{
    char* const pBufferEnd = pBuffer + bufSize;

    va_list args;
    va_start(args, format);

    size_t numCharsWritten = 0;

    const char* pFormatCursor = format;
    while (*pFormatCursor != '\0' && pBuffer < pBufferEnd - 1)
    {
        char formatChar = *pFormatCursor++;
        // char outputChar;
        if (formatChar == '%')
        {
            formatChar = *pFormatCursor++;
            switch (formatChar)
            {
            case '%':
                *pBuffer++ = '%';
                numCharsWritten += 1;
                break;

            case 'd':
                {
                    // Signed values?
                    auto value = va_arg(args, uint32_t);
                    auto numDigits = int_to_string<10>(pBuffer, pBufferEnd, value);
                    pBuffer += numDigits;
                    numCharsWritten += numDigits;
                    break;
                }


            case 'x':
            case 'X':
                {
                    // Signed values?
                    auto value = va_arg(args, uint32_t);
                    auto numDigits = int_to_string<16>(pBuffer, pBufferEnd, value);
                    pBuffer += numDigits;
                    numCharsWritten += numDigits;
                    break;
                }


            // TODO: Do I want to support float formatting, int padding, etc?
            // Certainly not at first.

            // Others: https://en.cppreference.com/w/cpp/io/c/fprintf

            default:
                *pBuffer++ = '?';
                numCharsWritten += 1;
                break;

            }
        }
        else
        {
            *pBuffer++ = formatChar;
            numCharsWritten += 1;
        }

    }
    *pBuffer = '\0';
    numCharsWritten += 1;

    va_end(args);
    return numCharsWritten;
}










}  // namespace stringio
