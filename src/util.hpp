#ifndef UTIL_H_
#define UTIL_H_


#include <cstdint>
#include <ostream>
#include <string>
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

constexpr u32 MASK5  = 0x1F;       // 0000...011111    (5 rightmost bits)
constexpr u32 MASK6  = 0x3F;       // 0000...111111    (6 rightmost bits)
constexpr u32 MASK16 = 0xFFFF;     // 0000...111111... (16 rightmost bits)
constexpr u32 MASK26 = 0x03FFFFFF; // 0000...111111... (26 rightmost bits)

std::string read_file_to_string(const char* filename);

struct SourceLocation {
    std::string_view file;
    size_t line, offset;
};

std::ostream& operator<<(std::ostream& os, const SourceLocation& loc);

#endif // UTIL_H_
