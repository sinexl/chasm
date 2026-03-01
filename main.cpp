#include "util.hpp"
#include "register.hpp"
#include <vector>

using namespace std;

class Format {
public:
    u32 encode() const;
};

template <const u8 op, // [6] Opcode
    const u8 funct>    // [6] function
class RFormat : public Format {
private:
    // opcode    [6]
    u8 rs;    // [5] first source
    u8 rt;    // [5] second source
    u8 rd;    // [5] destination
    u8 shamt; // [5] shift amount (shift ops)
    // funct     [6]
public:
    u32 encode() const
    {
        // Throughout the code masks are used to keep only last N bits of corresponding N bit field of instruction.
        // The numbers are "packed" into single 32 bit word like this:
        // Each field that is M bits to the right from LSB in MIPS32 instruction gets left shifted by M bits
        return
            (op    & MASK6) << 26 |
            (rs    & MASK5) << 21 |
            (rt    & MASK5) << 16 |
            (rd    & MASK5) << 11 |
            (shamt & MASK5) << 6  |
            (funct & MASK6);
    }

protected:
    RFormat(u8 rs, u8 rt, u8 rd, u8 shamt)
        : rs(rs)
        , rt(rt)
        , rd(rd)
        , shamt(shamt)
    {
    }
};

template <const u8 op /* [6] - opcode */>
class IFormat : public Format {
private:
    // opcode   [6]
    u8 rs;   // [5] source
    u8 rt;   // [5] destination/target
    u16 imm; // [16] immediate value

public:
    u32 encode() const
    {
        return
            (op  & MASK6) << 26 |
            (rs  & MASK5) << 21 |
            (rt  & MASK5) << 16 |
            (imm & MASK16);
    }

protected:
    IFormat(u8 rs, u8 rt, u16 imm) : rs(rs), rt(rt), imm(imm) {}
};

u8 reg_u8(Reg r) {
    return static_cast<u8>(r);
}

template <
    const u8 op // [6] - opcode
    >
class JFormat : public Format {
private:
    // opcode       [6]
    u32 address; // [26] address
    u32 encode() const
    {
        return (op      & MASK6) << 26
            |  (address & MASK26);
    }

public:
    explicit JFormat(u32 address)
        : address(address)
    { }
};

struct Add : RFormat<0x00, 0x06> {
public:
    Add(Reg rs, Reg rt, Reg rd)
        : RFormat(reg_u8(rs), reg_u8(rt), reg_u8(rd), 0)
    { }
};

struct Addi : IFormat<0b001000> {

    Addi(Reg rs, Reg rt, u16 imm)
        : IFormat(reg_u8(rs), reg_u8(rt), imm)
    { }
};

struct Function {
    vector<u32> code;

public:
    void add(Reg destination, Reg a, Reg b)
    {
        code.push_back(Add(a, b, destination).encode());
    }

    void addi(Reg destination, Reg a, u16 value) {
        code.push_back(Addi(a, destination, value).encode());
    }


};

int main()
{
    Function func {};
    func.addi(Reg::t0, Reg::zero, 10);
    return 0;
}
