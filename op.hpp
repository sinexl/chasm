#ifndef CHASM_OP_HPP
#define CHASM_OP_HPP
#include <unordered_map>
#include <string_view>

#include "register.hpp"
using namespace std;

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
    COUNT,
};
enum class IFormatInstruction: int {
    Addi,
    Addiu,
    COUNT,
};

using namespace reg;
static const std::unordered_map<string_view, RFormatInstruction> r_format_strings {
    {
        {"add", RFormatInstruction::Add},
        {"sub", RFormatInstruction::Sub},
        {"subu", RFormatInstruction::Subu},
        {"addu", RFormatInstruction::Addu},
        {"or", RFormatInstruction::Or},
        {"and", RFormatInstruction::And},
        {"jr", RFormatInstruction::Jr},
    }
};
static_assert(7 == static_cast<int>(RFormatInstruction::COUNT));

static  const std::unordered_map<string_view, IFormatInstruction> i_format_strings {
        {"addi", IFormatInstruction::Addi},
        {"addiu", IFormatInstruction::Addiu},
};
static_assert(2 == static_cast<int>(IFormatInstruction::COUNT));

//                                                    opcode, funct
static const std::unordered_map<RFormatInstruction, pair<u8, u8>> r_format_codes {
    {RFormatInstruction::Add,  {0b000000, 0b100000}},
    {RFormatInstruction::Addu, {0b000000, 0b100001}},
    {RFormatInstruction::Sub,  {0b000000, 0b100010}},
    {RFormatInstruction::Subu, {0b000000, 0b100011}},
    {RFormatInstruction::Or,   {0b000000, 0b100101}},
    {RFormatInstruction::And,  {0b000000, 0b100100}},
    {RFormatInstruction::Jr,   {0b000000, 0b001000}},
};
static_assert(7 == static_cast<int>(RFormatInstruction::COUNT));

//                                                  opcode
static const std::unordered_map<IFormatInstruction, u8> i_format_codes {
    {IFormatInstruction::Addi,  0b001000 },
    {IFormatInstruction::Addiu, 0b001001 },
};
static_assert(2 == static_cast<int>(IFormatInstruction::COUNT));

}
#endif //CHASM_OP_HPP
