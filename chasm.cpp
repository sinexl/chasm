#include <cassert>
#include <fstream>

#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string_view>
#include <vector>

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
            u8 b0 = (inst >> 24) & 0xFF; // MSB
            u8 b1 = (inst >> 16) & 0xFF;
            u8 b2 = (inst >> 8) & 0xFF;
            u8 b3 = inst & 0xFF; // LSB
            data.push_back(b0);
            data.push_back(b1);
            data.push_back(b2);
            data.push_back(b3);
        }
    }
};


class ParserException : std::exception
{
public:
    const char* what() const noexcept override = 0;
};

class UnexpectedToken : ParserException
{
    string msg_;

public:
    TokenType expected;
    TokenType actual;

    UnexpectedToken(TokenType expected, TokenType actual) : expected(expected), actual(actual)
    {
        ostringstream ss;
        ss << "error: unexpected token. Expected " << get_string(expected) << ", but got: " << get_string(actual);
        msg_ = ss.str();
    }

    const char* what() const noexcept override
    {
        return msg_.c_str();
    };
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

void parse_function(Lexer& lexer, Assembler& assembler)
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
        case TokenType::Register:
        case TokenType::Identifier:
        case TokenType::Comma:
        case TokenType::LabelDefinition:
            break;

        case TokenType::RFormatInstruction:
            {
                auto instruction = t.get_r_format();
                parse_r_format(lexer, assembler, instruction);
                break;
            }
        case TokenType::IFormatInstruction:
            assert(false && "NOT IMPLEMENTED YET");
        }
        if (stop) break;
    }
    while (!lexer.is_eof());
}


int main()
{
    using namespace op;
    assert(RFormat(RFormatInstruction::Add, Reg::t1, Reg::t2, Reg::t0, 0).encode() == 0x12a4020);
    assert(IFormat(IFormatInstruction::Addi ,Reg::s1, Reg::t0, -50).encode() == 0x2228ffce);
    const char* path = "main.asm";
    auto contents = read_file_to_string(path);

    auto view = string_view(contents);
    auto lexer = Lexer(view, path);
    auto assembler = Assembler{};
    try
    {
        parse_function(lexer, assembler);
    }
    catch (ParserException& e)
    {
        cerr << e.what() << endl;
    }

    auto result = vector<u8>{};
    assembler.assemble(result);
    assert(result.size() % 4 == 0);
    cout << "Decoding each word:" << endl;
    for (size_t i = 0; i < result.size(); i+=4)
    {
        u32 b0 = result[i],
            b1 = result[i + 1],
            b2 = result[i + 2],
            b3 = result[i + 3];

        u32 inst = 0;
        inst |= b0 << 24;
        inst |= b1 << 16;
        inst |= b2 << 8;
        inst |= b3;
        cout << std::hex << "0x" << inst << std::dec << endl;
    }
    {
        std::ofstream file{"./dev/output.bin", std::ios::binary};

        file.write(reinterpret_cast<char*>(result.data()), result.size());
    }


    return 0;
}
