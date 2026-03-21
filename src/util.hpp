#ifndef UTIL_H_
#define UTIL_H_


#include <cstdint>
#include <string>
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

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




///////////////////////////////////////////////////////////////////////////////
//                           BIG ENDIAN OPERATIONS                           //
///////////////////////////////////////////////////////////////////////////////


// Writing operations

#ifdef __cplusplus
#define CAST(Type, k) (static_cast<Type>((k)))
#else
#define CAST(Type, k) (Type)((k))
#endif

#ifdef __cplusplus
#define REINTERPRET(Type, k) (reinterpret_cast<Type>((k)))
#else
#define REINTERPRET(Type, k) (Type)((k))
#endif


// TODO: conditionally use compiler builtins wherever it is applicable

static inline void u16_to_be(u8 buffer[2], u16 value) {
    buffer[0] = CAST(u8, value >> 8); // [15:8] MSB
    buffer[1] = CAST(u8, value);      // [7:0]  LSB
}

static inline void u32_to_be(u8 buffer[4], u32 value) {
    buffer[0] = CAST(u8, value >> 24); // [31:24] MSB
    buffer[1] = CAST(u8, value >> 16); // [23:16]
    buffer[2] = CAST(u8, value >> 8);  // [15:8]
    buffer[3] = CAST(u8, value);       // [7:0]   LSB
}

static inline void u64_to_be(u8 buffer[8], u64 value) {
    buffer[0] = CAST(u8, value >> 56); // [63:56] MSB
    buffer[1] = CAST(u8, value >> 48); // [55:48]
    buffer[2] = CAST(u8, value >> 40); // [47:40]
    buffer[3] = CAST(u8, value >> 32); // [39:32]
    buffer[4] = CAST(u8, value >> 24); // [31:24]
    buffer[5] = CAST(u8, value >> 16); // [23:16]
    buffer[6] = CAST(u8, value >> 8);  // [15:8]
    buffer[7] = CAST(u8, value);       // [7:0]   LSB
}

static inline void i16_to_be(u8 buffer[2], i16 value) {
    u16_to_be(buffer, *REINTERPRET(u16*, &value));
}

static inline void i32_to_be(u8 buffer[4], i32 value) {
    u32_to_be(buffer, *REINTERPRET(u32*, &value));
}

static inline void i64_to_be(u8 buffer[8], i64 value) {
    u64_to_be(buffer, *REINTERPRET(u64*, &value));
}

static inline u16 u16_from_be(const u8 buffer[2]) {
    return CAST(u16,
        (CAST(u16, buffer[0]) << 8) | // [15:8]
         CAST(u16, buffer[1])         // [7:0]
    );
}

static inline u32 u32_from_be(const u8 buffer[4]) {
    return (CAST(u32, buffer[0]) << 24) | // [31:24]
           (CAST(u32, buffer[1]) << 16) | // [23:16]
           (CAST(u32, buffer[2]) << 8)  | // [15:8]
            CAST(u32, buffer[3]);         // [7:0]
}

static inline u64 u64_from_be(const u8 buffer[8]) {
    return (CAST(u64, buffer[0]) << 56) | // [63:56]
           (CAST(u64, buffer[1]) << 48) | // [55:48]
           (CAST(u64, buffer[2]) << 40) | // [47:40]
           (CAST(u64, buffer[3]) << 32) | // [39:32]
           (CAST(u64, buffer[4]) << 24) | // [31:24]
           (CAST(u64, buffer[5]) << 16) | // [23:16]
           (CAST(u64, buffer[6]) << 8)  | // [15:8]
            CAST(u64, buffer[7]);         // [7:0]
}

#endif // UTIL_H_
