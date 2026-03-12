#ifndef CHASM_OP_HPP
#define CHASM_OP_HPP
#include <unordered_map>
#include <string_view>

#include "format.hpp"
#include "register.hpp"
using namespace std;
#include "lexer.hpp"

namespace op
{
    const std::unordered_map<string_view, TokenType> tokens {
        {
            {"add", TokenType::Add},
            {"addi", TokenType::Addi},
            {"addu", TokenType::Addu},
            {"addiu", TokenType::Addiu},
            {"or", TokenType::Or},
            {"and", TokenType::And},
            {"j", TokenType::J},
            {"jr", TokenType::Jr}
        }
    };
    static_assert(8 == static_cast<int>(TokenType::COUNT) - static_cast<int>(TokenType::FIRST_INSTRUCTION) - 1);
    struct Add final : RFormat<0x00, 0b100000> { Add(Reg rd, Reg rs, Reg rt) : RFormat(reg_u8(rs), reg_u8(rt), reg_u8(rd), 0) { } };
    struct Addi final : IFormat<0b001000> { Addi(Reg rt, Reg rs, u16 imm) : IFormat(reg_u8(rs), reg_u8(rt), imm) { } };
    // Add but no trap on overflow
    struct Addu final : RFormat<0x00, 0b100001> { Addu(Reg rd, Reg rs, Reg rt) : RFormat(reg_u8(rs), reg_u8(rt), reg_u8(rd), 0) { } };
    struct Addiu final : IFormat<0b001001> { Addiu(Reg rt, Reg rs, u16 imm) : IFormat(reg_u8(rs), reg_u8(rt), imm) { } };

    struct Or final : RFormat<0x00, 0b100101> { Or(Reg rd, Reg rs, Reg rt) : RFormat(reg_u8(rs), reg_u8(rt), reg_u8(rd), 0) { } };
    struct And final : RFormat<0x00, 0b100100> { And(Reg rd, Reg rs, Reg rt) : RFormat(reg_u8(rs), reg_u8(rt), reg_u8(rd), 0) { } };

    struct J final : JFormat<0b000010> { explicit J(u32 address) : JFormat(address) { } };
    struct Jr final : RFormat<0x00, 0b0001000> { explicit Jr(Reg rs) : RFormat(reg_u8(rs), 0, 0, 0) { } };


    inline TokenType kind_from_word(const string_view& s)
    {
        const auto type = tokens.find(s);
        if (type == tokens.end())
            return TokenType::Identifier;
        return type->second;
    }
}
#endif //CHASM_OP_HPP