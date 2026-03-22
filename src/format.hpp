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
    virtual bool is_control_flow() const = 0;
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

    bool is_control_flow() const override
    {
        static_assert(8 == static_cast<int>(RFormatInstruction::COUNT));
        if (op == RFormatInstruction::Jr) return true;
        return false;
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
    std::variant<          // [16] immediate value
        u16,               // immediate value (including address)
        string             // unresolved label.
    > imm;

public:
    u32 encode() const override
    {
        auto code = i_format_codes.find(op);
        assert(code != i_format_codes.end());
        auto opcode = code->second;
        assert(holds_alternative<u16>(imm) && "ERROR: Instruction is not resolved to be encoded"); // TODO: Proper error handling
        u16 immediate = std::get<u16>(imm);
        return
            opcode << 26 |
                rs << 21 |
                rt << 16 |
                immediate;
    }
    bool is_control_flow() const override
    {
        static_assert(5 == static_cast<int>(IFormatInstruction::COUNT));
        if (op == IFormatInstruction::Bne) return true;
        if (op == IFormatInstruction::Beq) return true;
        return false;
    }

    IFormat(IFormatInstruction op, Reg rs, Reg rt, u8 bytes[2]) : op(op), rs(reg_u8(rs)), rt(reg_u8(rt))
    {
        u16 val = u16_from_be(bytes);
        imm = val;
    }

    void resolve(u8 bytes[2])
    {
        if (!holds_alternative<string>(imm))
        {
            assert(false && "Instruction is already resolved"); // TODO: Proper error handling
        }
        u16 result = u16_from_be(bytes);
        imm = result;

    }
    string get_label() const
    {
        if (!holds_alternative<string>(imm))
        {
            assert(false && "Instruction is already resolved");
        }
        return std::get<string>(imm);
    }

    IFormat(IFormatInstruction op, Reg rs, Reg rt, string label) : op(op), rs(reg_u8(rs)), rt(reg_u8(rt)), imm(std::move(label))
    {
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
