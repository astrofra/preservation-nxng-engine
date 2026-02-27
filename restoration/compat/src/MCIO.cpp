#include "IO/MCIO.h"

#include <cstdint>
#include <cstring>

unsigned long MCIO::GetLong(const char* p) const {
    const auto* b = reinterpret_cast<const unsigned char*>(p);
    return (static_cast<unsigned long>(b[0]) << 24U) |
           (static_cast<unsigned long>(b[1]) << 16U) |
           (static_cast<unsigned long>(b[2]) << 8U) |
           static_cast<unsigned long>(b[3]);
}

unsigned short MCIO::GetWord(const char* p) const {
    const auto* b = reinterpret_cast<const unsigned char*>(p);
    return static_cast<unsigned short>((static_cast<unsigned short>(b[0]) << 8U) |
                                       static_cast<unsigned short>(b[1]));
}

float MCIO::LongAsFloat(unsigned long v) const {
    const std::uint32_t bits = static_cast<std::uint32_t>(v);
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}
