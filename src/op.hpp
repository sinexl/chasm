#ifndef CHASM_OP_HPP
#define CHASM_OP_HPP
#include <unordered_map>
#include <string_view>

#include "register.hpp"

namespace op
{

enum class RFormatInstruction: int {
    Add,
    Addu,
    Sub,
    Subu,
    Or,
    And,
    Jr,
    Nop,
    COUNT,
};
enum class IFormatInstruction: int {
    Addi,
    Addiu,
    Slti,
    Bne,
    Beq,
    COUNT,
};

enum class JFormatInstruction: int {
    J,
    COUNT
};

///////////////////////////////////////////////////////////////////////////////
//                                  STRINGS                                  //
///////////////////////////////////////////////////////////////////////////////


using namespace reg;
static const std::unordered_map<std::string_view, RFormatInstruction> r_format_strings {
    {
        {"add", RFormatInstruction::Add},
        {"sub", RFormatInstruction::Sub},
        {"subu", RFormatInstruction::Subu},
        {"addu", RFormatInstruction::Addu},
        {"or", RFormatInstruction::Or},
        {"and", RFormatInstruction::And},
        {"jr", RFormatInstruction::Jr},
        {"nop", RFormatInstruction::Nop},
    }
};
static_assert(8 == static_cast<int>(RFormatInstruction::COUNT));

static const std::unordered_map<std::string_view, IFormatInstruction> i_format_strings {
        {"addi", IFormatInstruction::Addi},
        {"addiu", IFormatInstruction::Addiu},
        {"slti", IFormatInstruction::Slti},
        {"bne", IFormatInstruction::Bne},
        {"beq", IFormatInstruction::Beq},
};
static_assert(5 == static_cast<int>(IFormatInstruction::COUNT));

static const std::unordered_map<std::string_view, JFormatInstruction> j_format_strings {
        {"j", JFormatInstruction::J},
};
static_assert(1 == static_cast<int>(JFormatInstruction::COUNT));

///////////////////////////////////////////////////////////////////////////////
//                                  ENCODING                                 //
///////////////////////////////////////////////////////////////////////////////

//                                                    opcode, funct
static const std::unordered_map<RFormatInstruction, std::pair<u8, u8>> r_format_codes {
    {RFormatInstruction::Add,  {0b000000, 0b100000}},
    {RFormatInstruction::Addu, {0b000000, 0b100001}},
    {RFormatInstruction::Sub,  {0b000000, 0b100010}},
    {RFormatInstruction::Subu, {0b000000, 0b100011}},
    {RFormatInstruction::Or,   {0b000000, 0b100101}},
    {RFormatInstruction::And,  {0b000000, 0b100100}},
    {RFormatInstruction::Jr,   {0b000000, 0b001000}},
    {RFormatInstruction::Nop,  {0b000000, 0b000000}},
};
static_assert(8 == static_cast<int>(RFormatInstruction::COUNT));

//                                                  opcode
static const std::unordered_map<IFormatInstruction, u8> i_format_codes {
    {IFormatInstruction::Addi,  0b001000 },
    {IFormatInstruction::Addiu, 0b001001 },
    {IFormatInstruction::Slti,  0b001010 },
    {IFormatInstruction::Bne,   0b000101 },
    {IFormatInstruction::Beq,   0b000100 },
};
static_assert(5 == static_cast<int>(IFormatInstruction::COUNT));

static const std::unordered_map<JFormatInstruction, u8> j_format_codes {
    {JFormatInstruction::J,  0b000010  },
};
static_assert(1 == static_cast<int>(JFormatInstruction::COUNT));

}
#endif //CHASM_OP_HPP
