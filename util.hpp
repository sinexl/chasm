#ifndef UTIL_H_
#define UTIL_H_


#include <cstdint>
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

const u32 MASK5  = 0x1F;       // 0000...011111    (5 rightmost bits)
const u32 MASK6  = 0x3F;       // 0000...111111    (6 rightmost bits)
const u32 MASK16 = 0xFFFF;     // 0000...111111... (16 rightmost bits)
const u32 MASK26 = 0x03FFFFFF; // 0000...111111... (26 rightmost bits)


#endif // UTIL_H_
