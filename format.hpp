#ifndef CHASM_FORMAT_HPP
#define CHASM_FORMAT_HPP
#include <cassert>

#include "register.hpp"
#include "util.hpp"

class Item
{
public:
    virtual ~Item() = default;
};

class Label final : public Item
{
    string name;
};
class Format: public Item
{
public:
    ~Format() override = default;
    virtual u32 encode() const = 0;
};

inline std::ostream& operator<<(std::ostream& os, const Format& obj)
{
    os << "0x" << std::hex << obj.encode() << std::dec;
    return os;
}

template <const u8 op,    // [6] Opcode
          const u8 funct> // [6] function
class RFormat : public Format
{
    // opcode    [6]
    u8 rs;    // [5] first source
    u8 rt;    // [5] second source
    u8 rd;    // [5] destination
    u8 shamt; // [5] shift amount (shift ops)
    // funct     [6]
public:
    u32 encode() const override
    {
        // Throughout the code masks are used to keep only last N bits of corresponding N bit field of instruction.
        // The numbers are "packed" into single 32 bit word like this:
        // Each field that is M bits to the right from LSB in MIPS32 instruction gets left shifted by M bits
        return
            op << 26 |
            rs << 21 |
            rt << 16 |
            rd << 11 |
            shamt << 6 |
            funct;
    }

protected:
    RFormat(u8 rs, u8 rt, u8 rd, u8 shamt)
        : rs(rs)
          , rt(rt)
          , rd(rd)
          , shamt(shamt)
    {
        assert(rs < 32 && "RFormat: Register rs out of range");
        assert(rt < 32 && "RFormat: Register rt out of range");
        assert(rd < 32 && "RFormat: Register rd out of range");
        assert(shamt < 32 && "RFormat: Shift amount out of range");
        assert(funct < 64 && "RFormat: Function out of range");
    }
};

template <const u8 op /* [6] - opcode */>
class IFormat : public Format
{
    // opcode   [6]
    u8 rs;   // [5] source
    u8 rt;   // [5] destination/target
    u16 imm; // [16] immediate value

public:
    u32 encode() const override
    {
        return
            op << 26 |
            rs << 21 |
            rt << 16 |
            imm;
    }

protected:
    IFormat(u8 rs, u8 rt, u16 imm) : rs(rs), rt(rt), imm(imm)
    {
        assert(rs < 32 && "IFormat: Register rs out of range");
        assert(rt < 32 && "IFormat: Register rt out of range");
        assert(imm < UINT16_MAX && "IFormat: Immediate value out of range");
    }
};


template <
    const u8 op // [6] - opcode
>
class JFormat : public Format
{
    // opcode       [6]
    u32 address; // [26] address

public:
    explicit JFormat(u32 address)
        : address(address)
    {
        assert((address & ~MASK26) == 0 && "JFormat: Address value out of range");
    }

    u32 encode() const override
    {
        return op << 26
            | address;
    }
};

#endif //CHASM_FORMAT_HPP