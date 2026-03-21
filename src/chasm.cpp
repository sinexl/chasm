#include <array>
#include <cassert>
#include <fstream>
#include <bitset>
#include <iomanip>
#include <variant>

#include <iostream>
#include <memory>
#include <ostream>
#include <string_view>
#include <vector>

#include "exceptions.hpp"
#include "format.hpp"
#include "register.hpp"
#include "util.hpp"

#include "lexer.hpp"
#include "op.hpp"

using namespace std;
using namespace reg;

class Assembler
{
    vector<unique_ptr<Format>> ir;

public:
    void r_format(RFormatInstruction type, Reg dst, Reg a, Reg b)
    {
        ir.push_back(make_unique<RFormat>(type, a, b, dst, 0));
    }

    // Please notice that on ISA level functions
    // "i_format_u16" and "i_format_i16" don't differ (they generate the same code).
    // Both just take the big endian bytes from 'value' parameter.
    // Both functions are provided explicitly in order to make library interface easier to work from C.
    void i_format_u16(IFormatInstruction type, Reg dst, Reg src, u16 value)
    {
        u8 bytes[2];
        u16_to_be(bytes, value);
        ir.push_back(make_unique<IFormat>(type, src, dst, bytes));
    }

    void i_format_i16(IFormatInstruction type, Reg dst, Reg src, i16 value)
    {
        u8 bytes[2];
        i16_to_be(bytes, value);
        ir.push_back(make_unique<IFormat>(type, src, dst, bytes));
    }


    // Jump to address found in register.
    void jr(Reg dst)
    {
        ir.push_back(make_unique<RFormat>(RFormatInstruction::Jr, dst, Reg::zero, Reg::zero, 0));
    }


    void assemble(vector<u8>& data) const
    {
        for (auto& item : ir)
        {
            u32 inst = item->encode();
            // Big endian encoding
            u8 bytes[4];
            u32_to_be(bytes, inst);
            data.push_back(bytes[0]);
            data.push_back(bytes[1]);
            data.push_back(bytes[2]);
            data.push_back(bytes[3]);
        }
    }
};


Token expect(Lexer& lexer, TokenType expected)
{
    auto token = lexer.next_token();
    auto got_type = token.get_type();
    if (got_type != expected)
    {
        throw UnexpectedToken(expected, got_type);
    }

    return token;
}


void parse_r_format(Lexer& lexer, Assembler& assembler, RFormatInstruction r_format)
{
    Token dest = expect(lexer, TokenType::Register);

    if (r_format == RFormatInstruction::Jr)
    {
        assembler.jr(dest.get_reg());
        return;
    }

    expect(lexer, TokenType::Comma);

    Token a = expect(lexer, TokenType::Register);
    expect(lexer, TokenType::Comma);
    Token b = expect(lexer, TokenType::Register);
    assembler.r_format(r_format, dest.get_reg(), a.get_reg(), b.get_reg());
}

void parse_i_format(Lexer& lexer, Assembler& assembler, IFormatInstruction i_format)
{
    Reg dest = expect(lexer, TokenType::Register).get_reg();
    expect(lexer, TokenType::Comma);
    Reg source = expect(lexer, TokenType::Register).get_reg();
    expect(lexer, TokenType::Comma);
    Integer integer = expect(lexer, TokenType::Integer).get_integer();
    if (std::holds_alternative<i64>(integer))
    {
        i64 val = std::get<i64>(integer);
        if (!(INT16_MIN <= val && val <= INT16_MAX))
            throw StaticIntegerOverflow(integer);
        assembler.i_format_i16(i_format, dest, source, static_cast<i16>(val));
    }
    else if (std::holds_alternative<u64>(integer))
    {
        u64 val = std::get<u64>(integer);
        if (val > UINT16_MAX)
            throw StaticIntegerOverflow(integer);

        assembler.i_format_u16(i_format, dest, source, static_cast<u16>(val));
    }
}

void parse_program(Lexer& lexer, Assembler& assembler)
{
    bool stop = false;
    do
    {
        Token t = lexer.next_token();
        cout << t.get_source_location() << ": " << t << endl;

        switch (t.get_type())
        {
        case TokenType::Eof:
            stop = true;
            break;
        case TokenType::LabelDefinition:
            assert(false && "NOT IMPLEMENTED YET: LABELS");
            break;

        case TokenType::RFormatInstruction:
            {
                auto instruction = t.get_r_format();
                parse_r_format(lexer, assembler, instruction);
                break;
            }
        case TokenType::IFormatInstruction:
            {
                auto instruction = t.get_i_format();
                parse_i_format(lexer, assembler, instruction);
                break;
            }

        case TokenType::JFormatInstruction:
            assert(false && "NOT IMPLEMENTED YET: J FORMAT INSTRUCTIONS");

        case TokenType::Register:
        case TokenType::Identifier:
        case TokenType::Comma:
        case TokenType::Integer:
            throw UnexpectedToken(TokenType::RFormatInstruction, t.get_type());
        }
        if (stop) break;
    }
    while (!lexer.is_eof());
}


int main()
{
    using namespace op;
    assert(RFormat(RFormatInstruction::Add, Reg::t1, Reg::t2, Reg::t0, 0).encode() == 0x12a4020);
    u8 bytes[2];
    i16_to_be(bytes, -50);
    assert(IFormat(IFormatInstruction::Addi ,Reg::s1, Reg::t0, bytes).encode() == 0x2228ffce);
    const char* path = "./dev/main.asm";
    auto contents = read_file_to_string(path);

    auto view = string_view(contents);
    auto lexer = Lexer(view, path);
    auto assembler = Assembler{};
    try
    {
        parse_program(lexer, assembler);
    }
    catch (ParserException& e)
    {
        cerr << e.what() << endl;
        exit(-1);
    }

    auto result = vector<u8>{};
    assembler.assemble(result);
    assert(result.size() % 4 == 0);
    cout << "Decoding each word:" << endl;
    for (size_t i = 0; i < result.size(); i += 4)
    {
        u32 inst = u32_from_be(result.data() + i);
        cout << std::hex << "0x" << std::setfill('0') << std::setw(8) << inst << std::dec << " (0b" << std::bitset<32>(inst) << ")" << endl;
    }
    {
        std::ofstream file{"./dev/output.bin", std::ios::binary};

        file.write(reinterpret_cast<char*>(result.data()), result.size());
    }


    return 0;
}
