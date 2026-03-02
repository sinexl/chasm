#ifndef UTIL_H_
#define UTIL_H_


#include <cstdint>
#include <fstream>
#include <string>
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;

constexpr u32 MASK5  = 0x1F;       // 0000...011111    (5 rightmost bits)
constexpr u32 MASK6  = 0x3F;       // 0000...111111    (6 rightmost bits)
constexpr u32 MASK16 = 0xFFFF;     // 0000...111111... (16 rightmost bits)
constexpr u32 MASK26 = 0x03FFFFFF; // 0000...111111... (26 rightmost bits)


std::string read_file_to_string(const char* filename)
{
    std::ifstream stream{filename};
    std::string file_contents{std::istreambuf_iterator(stream), std::istreambuf_iterator<char>()};
    return file_contents;
}



#endif // UTIL_H_
