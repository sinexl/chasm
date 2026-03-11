#include <cassert>

#include "util.hpp"
#include "register.hpp"
#include <iostream>
#include <memory>
#include <ostream>
#include <string_view>
#include <vector>

#include "lexer.hpp"

using namespace std;

class Format
{
public:
    virtual ~Format() = default;
    virtual u32 encode() const = 0;
};

std::ostream& operator<<(std::ostream& os, const Format& obj)
{
    os << "0x" << hex << obj.encode() << dec;
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

u8 reg_u8(Reg r)
{
    return static_cast<u8>(r);
}

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

namespace op
{
    struct Add final : RFormat<0x00, 0b100000>
    {
        Add(Reg rd, Reg rs, Reg rt)
            : RFormat(reg_u8(rs), reg_u8(rt), reg_u8(rd), 0)
        {
        }
    };

    struct Addi final : IFormat<0b001000>
    {
        Addi(Reg rt, Reg rs, u16 imm)
            : IFormat(reg_u8(rs), reg_u8(rt), imm)
        {
        }
    };

    struct J final : JFormat<0b000010>
    {
        J(u32 address)
            : JFormat(address)
        { }
    };
}

struct Function
{
    vector<unique_ptr<Format>> instructions;

    op::Add add(Reg destination, Reg a, Reg b)
    {
        auto add = op::Add{destination, a, b};
        instructions.push_back(make_unique<op::Add>(add));
        return add;
    }

    void addi(Reg destination, Reg a, u16 value)
    {
        instructions.push_back(make_unique<op::Addi>(destination, a, value));
    }

    [[nodiscard]] vector<u8> compile() const
    {
        vector<u8> result;
        for (auto& i : instructions)
        {
            u32 code = i->encode();
            const auto bytes = reinterpret_cast<u8*>(&code);
            result.insert(result.end(), bytes, bytes + 4);
        }

        return result;
    }
};

int main()
{
    using namespace op;
    Function func{};
    assert(Add(Reg::t0, Reg::t1, Reg::t2).encode() == 0x12a4020);
    assert(Addi(Reg::t0, Reg::s1, -50).encode() == 0x2228ffce);
    const char* path = "main.asm";
    auto contents = read_file_to_string(path);

    auto view = string_view(contents);
    auto lexer = Lexer(view, path);

    do
    {
        Token t = lexer.next_token();
        cout << t << endl;

    } while (!lexer.is_eof());




    return 0;
}
