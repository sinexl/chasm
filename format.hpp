#ifndef CHASM_FORMAT_HPP
#define CHASM_FORMAT_HPP
#include <cassert>

#include "register.hpp"
#include "util.hpp"
#include "op.hpp"

using namespace op;

class AsmItem
{
public:
    virtual ~AsmItem() = default;
};

class Label final : public AsmItem
{
    std::string name;
};
class Format: public AsmItem
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

class RFormat : public Format
{
    RFormatInstruction op; // [6]
    u8 rs;                 // [5] first source
    u8 rt;                 // [5] second source
    u8 rd;                 // [5] destination
    u8 shamt;              // [5] shift amount (shift ops)
    // funct                  [6]
public:
    u32 encode() const override
    {
        // Throughout the code masks are used to keep only last N bits of corresponding N bit field of instruction.
        // The numbers are "packed" into single 32 bit word like this:
        // Each field that is M bits to the right from LSB in MIPS32 instruction gets left shifted by M bits
        auto codes_ptr = r_format_codes.find(op);
        assert(codes_ptr != r_format_codes.end());

        auto codes = codes_ptr->second;
        auto opcode = codes.first;
        auto funct = codes.second;
        return
            opcode << 26 |
            rs << 21 |
            rt << 16 |
            rd << 11 |
            shamt << 6 |
            funct;
    }

    RFormat(RFormatInstruction op, Reg rs, Reg rt, Reg rd, u8 shamt)
        : op(op),
          rs(reg_u8(rs))
          , rt(reg_u8(rt))
          , rd(reg_u8(rd))
          , shamt(shamt)
    {
        assert(shamt < 32 && "RFormat: Shift amount out of range");
    }
};

class IFormat : public Format
{
    IFormatInstruction op; // [6] op
    u8 rs;                 // [5] source
    u8 rt;                 // [5] destination/target
    u16 imm;               // [16] immediate value

public:
    u32 encode() const override
    {
        auto code = i_format_codes.find(op);
        assert(code != i_format_codes.end());
        auto opcode = code->second;
        return
            opcode << 26 |
                rs << 21 |
                rt << 16 |
                imm;
    }

    IFormat(IFormatInstruction op, Reg rs, Reg rt, u16 imm) : op(op), rs(reg_u8(rs)), rt(reg_u8(rt)), imm(imm)
    {
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
